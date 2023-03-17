#include <iostream>
#include <stdlib.h>
#include <unistd.h>
using namespace std;

#include "stack.h"
#define PUSH 0
#define POP 1

thread_local int tid=-1;
/*
 * try_elim_array
 * Threads which failed CAS try to combine using elimination array
 */
bool sgl_stack::try_elim_array(tinfo *my_tinfo)
{
     // get random slot
    int rand_tid=rand()%get_num_threads();
    elim_arr[tid].store(my_tinfo);
 
    int other=tid_arr[rand_tid].load(); // get other thread tid
    
    while(!tid_arr[rand_tid].compare_exchange_weak(other,tid,memory_order_seq_cst)); //overwrite with your tid
    
    if (other!=-1)
    {
        //try combining with thread occupying this location
        tinfo *t_ptr=elim_arr[other].load(memory_order_seq_cst);
        
        if(t_ptr!=NULL && t_ptr->tid==other && t_ptr->op!=my_tinfo->op)
        {
            // can combine
            
            tinfo *tmp_myinfo;
            tmp_myinfo=my_tinfo;       
            // eliminate yourself first
            if(elim_arr[tid].compare_exchange_weak(tmp_myinfo,NULL,memory_order_seq_cst))
            {
                
                if(my_tinfo->op==PUSH)  // give the other thread your value if you are PUSH
                {
                    if(elim_arr[other].compare_exchange_strong(t_ptr,my_tinfo,memory_order_seq_cst))
                    {
                        return true;
                    }
                }
                else  //take the other thread value if you are POP
                {
                    if(elim_arr[other].compare_exchange_weak(t_ptr,NULL,memory_order_seq_cst))
                    {
                        //cout << "outside slot" <<endl;
                        my_tinfo->val=t_ptr->val;
                        return true; 
                    }
                }
                return false; 
            }
            
        }
    }// end of other != -1
    
    usleep(20);
    
    tinfo *tmp_myinfo;
    tmp_myinfo=my_tinfo;
    if(!elim_arr[tid].compare_exchange_strong(tmp_myinfo,NULL,memory_order_seq_cst)) // remove self 
    {
         if(my_tinfo->op==POP) // if cas fails then someone joined
         {
                 my_tinfo->val=tmp_myinfo->val;
         }
         return true;
     }
    return false;

}

void sgl_stack::push(int num) 
{
        if (tid==-1)
            tid=thread_count.fetch_add(1,memory_order_seq_cst);
        
        tinfo *my_tinfo=new tinfo;
        my_tinfo->val=num;
        my_tinfo->op=PUSH;
        my_tinfo->tid=tid;
        bool success=false;
        while(!success)
        {
            success=lk.try_lock();
            if(success)break;
            
            if(!elim_disabled())
            {
                success=try_elim_array(my_tinfo); // cas failed, now try elimination array
                if(success)
                    return;
                
            } 
        }
        stack.push_back(num); 
        lk.unlock();
}

int sgl_stack::pop()
{
        if (tid==-1)
            tid=thread_count.fetch_add(1,memory_order_seq_cst);

        int ret=-1;
        tinfo *my_tinfo = new tinfo;
        my_tinfo->op=POP;
        my_tinfo->tid=tid;
        bool success=false;
    
        while(!success)
        {
            success=lk.try_lock();
            if(success)break;
            if(!elim_disabled())
            {
                success=try_elim_array(my_tinfo); // cas failed, now try elimination array
                if(success){
                    return my_tinfo->val;
                }
            } 
        }    
        if(stack.size()>0)
        {
            ret=stack.back();
            stack.pop_back();
        }
        lk.unlock();
        return ret;
}


void triber_stack::push(int num) 
{
    if (tid==-1)
        tid=thread_count.fetch_add(1,memory_order_seq_cst);

    tinfo *my_tinfo=new tinfo;
    my_tinfo->val=num;
    my_tinfo->op=PUSH;
    my_tinfo->tid=tid;
    
    struct node *node = new struct node(num);
    struct node *t;
    
    bool success=false;
    
    while(!success)
    {
    
        t=top.load(memory_order_seq_cst); // load top
        node->next.store(t,memory_order_seq_cst); // make top my next node

        if(top.compare_exchange_strong(t,node,memory_order_seq_cst)) // cas self as top
            return;
        
        if(!elim_disabled())
        {
            success=try_elim_array(my_tinfo); // cas failed, now try elimination array
            if(success)return ;

        }
    }

        
    
}

int triber_stack::pop()
{
    if (tid==-1)
        tid=thread_count.fetch_add(1,memory_order_seq_cst);

    int ret=-1;
    tinfo *my_tinfo = new tinfo;
    my_tinfo->op=POP;
    my_tinfo->tid=tid;
    
    struct node *t,*n;
    int val;
    bool success=false;
    
    while(!success)
    {
        t=top.load(memory_order_seq_cst);
        if(t==NULL)return -1;
        n=t->next.load(memory_order_seq_cst);
        val=t->val.load(memory_order_seq_cst);   
        
        if(top.compare_exchange_strong(t,n,memory_order_seq_cst))  // cas with next of top
            break;
        if(!elim_disabled())
        {
            success=try_elim_array(my_tinfo); // cas failed, now try elimination array
            if(success){
                return my_tinfo->val;
            }
        }
    }
    delete t;
    return val;
}
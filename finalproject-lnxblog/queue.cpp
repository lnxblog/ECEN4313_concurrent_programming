#include "queue.h"

void sgl_queue::enqueue(int val)
{
    lk.lock();
    q.push(val);
    lk.unlock();
    
}


int sgl_queue::dequeue()
{
    int ret;
    lk.lock();
    if (q.empty())
        ret=-1;
    else
    {
        ret=q.front();
        q.pop();
    }
    lk.unlock();
    return ret;
}

void msqueue::enqueue(int val)
{
    
    struct qnode *curr_t,*next;
    struct qnode* ins = new struct qnode;
    ins->val=val;
    
    while(true)
    {
        curr_t=tail.load(memory_order_seq_cst);  // get current tail
        next=curr_t->next.load(memory_order_seq_cst); //get next of tail
        if(curr_t==tail.load()) // check tail again
        {
            if(next==NULL && curr_t->next.compare_exchange_strong(next,ins,memory_order_seq_cst)) // if next null then cas and append self
                break;
            else if(next!=NULL)tail.compare_exchange_strong(curr_t,next,memory_order_seq_cst); // else if next not null then move tail
        }       
    }
    tail.compare_exchange_strong(curr_t,ins); // set tail to self after appending
}

int msqueue::dequeue()
{
    struct qnode *curr_h,*curr_t,*next;
    
    while (true)
    {
        curr_h = head.load(memory_order_seq_cst);
        curr_t = tail.load(memory_order_seq_cst);
        next = curr_h->next.load();
        
        if(curr_h==head.load()) // get current head
        {
            if(curr_h==curr_t) // if tail and head point to dummy node
            {
                if(next==NULL) // queue empty
                    return -1;
                else
                    tail.compare_exchange_strong(curr_t,next,memory_order_seq_cst); // not empty, update tail
            }
            else
            {
                int ret=next->val; // head node is dummy, dequeue from next node after dummy
                
                if(head.compare_exchange_strong(curr_h,next,memory_order_seq_cst))// update dummy node to next node 
                   return ret;   
            }       
        }
    }
}
#include <iostream>
#include <deque>
#include <mutex>
#include <vector>
#include <atomic>
using namespace std;

typedef struct{
 int tid;
 char op;
 int val;
}tinfo;

// elimination array
// initialize with  nulls
// thread comes in and gets random location in elim array
// if not opposite op retry some other location
// if opposite op then cas the location based on push or pop
class sgl_stack {
    
    mutex lk;
    deque<int> stack;
    int disable_elim;
    atomic <tinfo*> *elim_arr;
    atomic <int> *tid_arr;
    atomic<int> thread_count;
    int num_threads;
    public:
    sgl_stack(int threads,int disable){
        num_threads=threads;
        disable_elim=disable;
        elim_arr = new atomic<tinfo*>[threads];
        tid_arr = new atomic<int>[threads-1];
        for(int i=0;i<threads-1;i++)
            tid_arr[i]=-1;
    }
    virtual void push(int);    
    virtual int pop();
    bool try_elim_array(tinfo*);
    

    int get_num_threads(){
        return num_threads-1;
    }
    
    bool elim_disabled()
    {
        return disable_elim==1;
    }
};


struct node{
    atomic<int> val;
    atomic<struct node*> next;
    node(int v)
    {val.store(v);}
};

class triber_stack: public sgl_stack {
    
    atomic <struct node*> top;
    atomic<int> thread_count;
    
    public:
    
    triber_stack(int threads,int disable): sgl_stack(threads,disable){
        top.store(NULL);
    }
    
    void push(int);    
    int pop();

};
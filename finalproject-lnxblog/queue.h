#include <iostream>
#include <queue>
#include <mutex>
#include <vector>
#include <atomic>
using namespace std;


// elimination array
// initialize with  nulls
// thread comes in and gets random location in elim array
// if not opposite op retry some other location
// if opposite op then cas the location based on push or pop
class sgl_queue {
    
    mutex lk;
    queue<int> q;

    atomic<int> thread_count;
    int num_threads;
    public:
    sgl_queue(){
    }
    virtual void enqueue(int);    
    virtual int dequeue();

};


struct qnode{
    int val;
    atomic<struct qnode*> next;
};
class msqueue: public sgl_queue{
    
    atomic<struct qnode*> head;
    atomic<struct qnode*> tail;
    public:
    msqueue(){
        struct qnode* dummy = new struct qnode;
        head.store(dummy);
        tail.store(dummy);
    }
    
    void enqueue(int);
    int dequeue();
        
};
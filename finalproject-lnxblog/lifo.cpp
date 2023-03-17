#include "lifo.h"

/******** MCS LOCK ******/
thread_local struct mcs_node my_node;
void mcs_lock::lock()
{
    my_node.next.store(NULL,memory_order_relaxed);  // initialize this thread my_node
    my_node.my_turn.store(false);
    struct mcs_node* oldTail;
    oldTail=tail.load(memory_order_seq_cst);   // get old tail
    
    while(!tail.compare_exchange_weak(oldTail,&my_node,memory_order_seq_cst));  // set tail to my_node
                                                                                // while tail!=oldtail, keep checking
                                                                                // this operation overwrites oldtail
    if(oldTail==NULL)
    {
        // we have the lock
    }
    else
    {
        oldTail->next.store(&my_node,memory_order_seq_cst);  // append my_node to oldtail
        while(!my_node.my_turn.load(memory_order_seq_cst));  // wait until oldtail gives you the lock
    
    }
}

void mcs_lock::unlock()
{
    struct mcs_node *ref=&my_node;
    if(!tail.compare_exchange_weak(ref,NULL,memory_order_seq_cst)) // check if you are not the tail
    {
        while(my_node.next.load(memory_order_seq_cst)==NULL);   // wait for the next node to update your next pointer
        struct mcs_node *next_node=my_node.next.load(memory_order_seq_cst);  // load the next node
        next_node->my_turn.store(true,memory_order_seq_cst);                // give the next node your lock
    }
}

/*********TAS LOCK*********/
tas_lock::tas_lock()
{
    tas_flag.clear();
}

void tas_lock::lock()
{
    while(tas_flag.test_and_set(memory_order_seq_cst)==true);
}

void tas_lock::unlock()
{
    tas_flag.clear(memory_order_seq_cst);
}



/******** MCS STACK LOCK ******/

void mcs_lock_s::lock()
{
    my_node.next.store(NULL,memory_order_relaxed);  // initialize this thread my_node
    my_node.my_turn.store(false);
    struct mcs_node* oldTop;
 
    do{
        
        oldTop=top.load(memory_order_seq_cst);   // get old top
        my_node.next.store(oldTop,memory_order_seq_cst); // set your next as old top
        
    }while(!top.compare_exchange_weak(oldTop,&my_node,memory_order_seq_cst)); // cas top to set yourself as top
    
    if(oldTop==NULL)
    {
        // we have the lock
    }
    else
    {
        while(!my_node.my_turn.load(memory_order_seq_cst));  // wait until current top gives you the lock    
    }
}

void mcs_lock_s::unlock()
{
    struct mcs_node *ref=&my_node;
    struct mcs_node *ref_next=ref->next.load(memory_order_seq_cst);
    struct mcs_node *oldTop,*oTnext;
    
    if(!top.compare_exchange_strong(ref,ref_next,memory_order_seq_cst))  // cas and check if you are not top 
    {
        /* more threads have arrived */
        oldTop=ref; 
        
        /* remove yourself by connecting previous to next */
        while(oldTop->next.load(memory_order_seq_cst)!=&my_node) 
            oldTop=oldTop->next.load(memory_order_seq_cst);
        oldTop->next.store(my_node.next.load(memory_order_seq_cst),memory_order_seq_cst);

        /* give top node which is stored in ref the lock */
        ref->my_turn.store(true,memory_order_seq_cst);
        
    }
    else{
        /* no other threads arrived. check if more nodes follow and provide lock to them */
        if(ref_next!=NULL)
            ref_next->my_turn.store(true,memory_order_seq_cst);
        
    }

}


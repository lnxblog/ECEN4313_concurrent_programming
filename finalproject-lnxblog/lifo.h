#include <atomic>
using namespace std;

class BaseLock{
	mutex base_lk;
    public:
     virtual void lock(){
     	base_lk.lock();
     }
     virtual void unlock(){
     	base_lk.unlock();
     }

};

struct mcs_node {
    atomic <mcs_node*> next;
    atomic <bool> my_turn;
};

class mcs_lock: public BaseLock{
    
    atomic <struct mcs_node*> tail=NULL;

    public:
    void lock();
    void unlock();
};

class tas_lock: public BaseLock{
 
    atomic_flag tas_flag;
    public:
    tas_lock();
    void lock();
    void unlock();
};

class mcs_lock_s: public BaseLock{
    
    atomic <struct mcs_node*> top=NULL;
    public:
    void lock();
    void unlock();
};
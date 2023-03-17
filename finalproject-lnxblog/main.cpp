#include <barrier>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <ctime>
#include <getopt.h>
#include <unistd.h>
#include <fstream>
using namespace std;


#include "stack.h"

vector<thread*> threads;

size_t NUM_THREADS=4;
#define AUTHOR "HEMANTH"


barrier<> *bar; 
sgl_stack *stk;
vector<int> pop_res;
struct timespec startTime, endTime;

/*
Function: global_init
Initialize synchronization primitives like barriers, locks
*/
void global_init()
{
	bar = new barrier(NUM_THREADS);
}

/*
Function: global_cleanup
Delete synchronization primitives like barriers, locks
*/
void global_cleanup(){
	delete bar;
}

void print_author()
{
    	cout << AUTHOR << endl;
}

void print_help()
{
    	cout << "Usage: stack_test [--name] [-t num_threads] [--stack=<sgl,treiber>] [-d disable elimination]" << endl;
}

/*
 * thread_stack_test
 * Spawn multiple threads to perform push and pop concurrently
 */

void thread_stack_test(int tid,int N)
{
    int start=N*tid;
    int i=0;
    vector<int> res;
    
    
    if(tid==0)
    {
        pop_res.resize(NUM_THREADS*N,0);
        clock_gettime(CLOCK_MONOTONIC,&startTime);
    }
    bar->arrive_and_wait();
    while(i<N)
    {
        stk->push(start+i);
        i++;
    }
    i=0;
    while(i<N)
    {
        int ret=stk->pop();
        pop_res[start+i]=ret;
       //cout <<"tid:"<<tid<<" "<< ret << endl;
        i++;
        //
    }    
    bar->arrive_and_wait();   
    if(tid==0)
       clock_gettime(CLOCK_MONOTONIC,&endTime); 
    
}

/*
* stack_test2
* Multithread access to stack
*/
int stack_test2()
{
    int iters=100000;
	for(int i=0; i<NUM_THREADS; i++)
	{
		threads[i] = new thread(thread_stack_test,i,iters);
	}
    	/* join spawned threads */
	for(int i=0; i<NUM_THREADS; i++)
    {
		threads[i]->join();
		printf("joined thread %d\n",i);
		delete threads[i];
	}
    
    /* check if the poo returned all the elements */
    sort(pop_res.begin(),pop_res.end());
    for(int i=0;i<iters*NUM_THREADS;i++)
    {
        if(i!=pop_res[i])
            return -1;
    }
    return 0;
        //cout << pop_res[i] << endl;
}

/* stack_test1
    Test if stack works in lifo manner
*/
int stack_test1()
{
    vector<int> res;
    int N=100000,ret=0;
    
    clock_gettime(CLOCK_MONOTONIC,&startTime);
    
    // push numbers 0-N and pop them
    for(int i=0;i<=N;i++)
        stk->push(i);
    for(int i=0;i<=N;i++)
        res.push_back(stk->pop());
    
    // check for lifo correctness
    for(int i=0;i<=N;i++)
    {
        if(res[i]!=N-i)
        {
            cout << "stack functionality failed" << endl;
            ret=-1;
            break;
        }
    }
    clock_gettime(CLOCK_MONOTONIC,&endTime);
    return ret;
}
/*
Function: parse_opts

Parse user provided options and return them in reference variables
*/
void parse_opts(int argc,char *argv[],int &num_threads,string &stack_str,int &disable_elim)
{

    	int c,option_index = 0;
 
    	static struct option long_options[] = 
    	{
    		/* define long options and value to return when option found */
        	{"name",no_argument, NULL,'n'},
            {"stack",required_argument,NULL,'s'},
        	{NULL,0, NULL,0}
    	};

    	while(1)
    	{
        	c = getopt_long(argc, argv, "-:s:t:d", long_options, &option_index);
        
        	if(c==-1)
            	break;
        
        	switch(c)
        	{
            		case 'n': print_author();
                    		break;
            		case 't': num_threads = stoi(optarg);
                    		break;
                    case 's': stack_str = optarg;
                            break;
                    case 'd': disable_elim = 1;
                            break;
            		case '?':
            			default: print_help();
        	}
    	}   
}

void print_time()
{
   	/* Print time taken */
	unsigned long long elapsed_ns;
	elapsed_ns = (endTime.tv_sec-startTime.tv_sec)*1000000000 + (endTime.tv_nsec-startTime.tv_nsec);
	printf("Elapsed (ns): %llu\n",elapsed_ns);
	double elapsed_s = ((double)elapsed_ns)/1000000000.0;
	printf("Elapsed (s): %lf\n",elapsed_s); 
    
}
/*
Function: main
*/
int main(int argc, char* argv[])
{
	
		int num_threads=0,disable_elim=0;
		string stack_str;

    
		if (argc<2)
    	{
        	print_help();
        	return 1;
    	}
    
    	parse_opts(argc,argv,num_threads,stack_str,disable_elim);
    
    if(num_threads!=0)	
    	NUM_THREADS=num_threads;
    
    global_init();
	// check stack option
	if(stack_str.compare("treiber")==0)
	{
		stk = new triber_stack(NUM_THREADS,disable_elim);
	}
	else
	{
		stk = new sgl_stack(NUM_THREADS,disable_elim);
	}

    threads.resize(NUM_THREADS);
    
    int ret;
    ret=stack_test1();
    if(ret!=0)
        cout << "stack_test1 failed" << endl;
    print_time();
    
    ret=stack_test2();
    if(ret!=0)
        cout << "stack_test2 failed" << endl;
    print_time();
	global_cleanup();
	
}

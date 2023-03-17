#include <barrier>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <ctime>
#include <getopt.h>
#include <unistd.h>
#include <fstream>
#include <atomic>
using namespace std;


#include "queue.h"

vector<thread*> threads;

size_t NUM_THREADS=4;
#define AUTHOR "HEMANTH"


barrier<> *bar; 
sgl_queue *que;
vector<int> pop_res;
atomic<int> idx=1,idx2=0;
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
    	cout << "Usage: queue_test [--name] [-t num_threads] [--queue=<sgl,ms>]" << endl;
}

/*
 * thread_queue_test
 * Spawn multiple threads to perform enqueue and dequeue concurrently
 */

void thread_queue_test(int tid,int N)
{
    int start=N*tid;
    int i=0;
    
    if(tid==0)
    {
        pop_res.resize(NUM_THREADS*N,0);
        clock_gettime(CLOCK_MONOTONIC,&startTime);
    }
    bar->arrive_and_wait();
    while(i<N)
    {
        que->enqueue(start+i);
        i++;
    }
    i=0;
    while(i<N)
    {
        int ret=que->dequeue();
        pop_res[start+i]=ret;
        i++;
    }     
    bar->arrive_and_wait();   
    if(tid==0)
       clock_gettime(CLOCK_MONOTONIC,&endTime); 
    
}

/*
* queue_test2
* Multithread access to queue
*/

int queue_test2()
{
    int iters=10;
	for(int i=0; i<NUM_THREADS; i++)
	{
		threads[i] = new thread(thread_queue_test,i,iters);
	}
    	/* join spawned threads */
	for(int i=0; i<NUM_THREADS; i++)
    {
		threads[i]->join();
		printf("joined thread %d\n",i);
		delete threads[i];
	}

    /* check if the dequeue returned all the elements */
    sort(pop_res.begin(),pop_res.end());
    for(int i=0;i<iters*NUM_THREADS;i++)
    {
        if(i!=pop_res[i])
            return -1;
    }
    return 0;
}

/* queue_test1
    Test if queue works in fifo manner
*/
int queue_test1()
{
    vector<int> res;
    int N=100,ret=0;
    
    clock_gettime(CLOCK_MONOTONIC,&startTime);
    
    // push numbers 0-N and pop them
    for(int i=0;i<=N;i++)
        que->enqueue(i);
    for(int i=0;i<=N;i++)
        res.push_back(que->dequeue());
    
    // check for fifo correctness
    for(int i=0;i<=N;i++)
    {
        if(res[i]!=i)
        {
            cout << "queue functionality failed" << endl;
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
void parse_opts(int argc,char *argv[],int &num_threads,string &queue_str,int &disable_elim)
{

    	int c,option_index = 0;
 
    	static struct option long_options[] = 
    	{
    		/* define long options and value to return when option found */
        	{"name",no_argument, NULL,'n'},
            {"queue",required_argument,NULL,'q'},
        	{NULL,0, NULL,0}
    	};

    	while(1)
    	{
        	c = getopt_long(argc, argv, "-:q:t:", long_options, &option_index);
        
        	if(c==-1)
            	break;
        
        	switch(c)
        	{
            		case 'n': print_author();
                    		break;
            		case 't': num_threads = stoi(optarg);
                    		break;
                    case 'q': queue_str = optarg;
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
		string queue_str;

    
		if (argc<2)
    	{
        	print_help();
        	return 1;
    	}
    
    	parse_opts(argc,argv,num_threads,queue_str,disable_elim);
    
    if(num_threads!=0)	
    	NUM_THREADS=num_threads;
    
    global_init();
	// check stack option
	if(queue_str.compare("ms")==0)
	{
		que = new msqueue();
	}
	else
	{
		que = new sgl_queue();
	}

    threads.resize(NUM_THREADS);
    
    int ret;
    ret=queue_test1();
    if(ret!=0)
        cout << "queue_test1 failed" << endl;
    print_time();

    ret=queue_test2();
    if(ret!=0)
        cout << "queue_test2 failed" << endl;
    print_time();
    
	global_cleanup();
	

    
}

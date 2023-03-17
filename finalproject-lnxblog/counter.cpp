#include <barrier>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <ctime>
#include <getopt.h>
#include <unistd.h>
#include <fstream>
#include <mutex>
using namespace std;


#include "lifo.h"

#define AUTHOR "HEMANTH"

vector<thread*> threads;

size_t NUM_THREADS=4;
BaseLock *lk;
barrier<> *bar;
struct timespec startTime, endTime;

/*
Function: global_init
Initialize synchronization primitives like barriers, locks
*/
void global_init()
{
	bar = new barrier(NUM_THREADS);
	//sbar = new sense_bar(NUM_THREADS);
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
    	cout << "Usage: counter [--name] [-t NUM_THREADS] [-i NUM_ITERATIONS] [--lock=<tas,lifo,mcs>] [-o out.txt]" << endl;
}
long int cntr=0;
/*
Function: write_output_file
Write counter value to output file
*/
void write_output_file(string output_file)
{
	ofstream fp;    
    fp.open(output_file);

    if(fp.is_open())
    {
        fp << cntr << endl;
        fp.close();
    }
}


void thread_main_lock(int tid,int num_iters)
{
    bar->arrive_and_wait();
    if(tid==1)
        clock_gettime(CLOCK_MONOTONIC,&startTime);
    
    for(int i = 0; i<num_iters; i++){
        lk->lock();
        cntr++;
        lk->unlock();
    }
    
    bar->arrive_and_wait();
    if(tid==1)
        clock_gettime(CLOCK_MONOTONIC,&endTime);
}


/*
Function: parse_opts

Parse user provided options and return them in reference variables
*/
void parse_opts(int argc,char *argv[],int &num_threads,int &num_iters,string &output_file,string &bar_str,string &lock_str)
{

    	int c,option_index = 0;
 
    	static struct option long_options[] = 
    	{
    		/* define long options and value to return when option found */
        	{"name",no_argument, NULL,'n'},
            {"lock",required_argument,NULL,'l'},
        	{NULL,0, NULL,0}
    	};

    	while(1)
    	{
        	c = getopt_long(argc, argv, "-:o:t:i:", long_options, &option_index);
        
        	if(c==-1)
            	break;
        
        	switch(c)
        	{
            		case 'n': print_author();
                    		break;
            		case 'i': num_iters=stoi(optarg);
                    		break;
            		case 'o': output_file = optarg;  
                    		break;
            		case 't': num_threads = stoi(optarg);
                    		break;
                    case 'l': lock_str = optarg;
                            break;
            		case '?':
            			default: print_help();
        	}
    	}   
}

void start_counter_lock(int num_iters)
{
    

    for(int i=0; i<NUM_THREADS; i++)
    {
        threads[i] = new thread(thread_main_lock,i+1,num_iters);
    }

    /* join spawned threads */
    for(size_t i=1; i<NUM_THREADS; i++)
    {
        threads[i]->join();
        printf("joined thread %zu\n",i+1);
        delete threads[i];
    }   
}

/*
Function: main
*/
int main(int argc, char* argv[])
{

    int num_threads=0,num_iters=1000;
    string bar_str,lock_str,output_file;

    if (argc<2)
    {
        print_help();
        return 1;
    }

    parse_opts(argc,argv,num_threads,num_iters,output_file,bar_str,lock_str);
       
    if (output_file.empty())
    {
        output_file="counter.log";
    }

    if(num_threads!=0)	
        NUM_THREADS=num_threads;
    
    global_init();

    threads.resize(NUM_THREADS);
    
    if(lock_str.compare("tas")==0)
        lk=new tas_lock;
    else if(lock_str.compare("mcs")==0)
        lk=new mcs_lock;
    else if(lock_str.compare("lifo")==0)
        lk=new mcs_lock_s;

    start_counter_lock(num_iters);
        

    /* write result to file */
    write_output_file(output_file);
    cout << "COUNTER: " << cntr << endl;
    global_cleanup();

    /* Print time taken */
    unsigned long long elapsed_ns;
    elapsed_ns = (endTime.tv_sec-startTime.tv_sec)*1000000000 + (endTime.tv_nsec-startTime.tv_nsec);
    printf("Elapsed (ns): %llu\n",elapsed_ns);
    double elapsed_s = ((double)elapsed_ns)/1000000000.0;
    printf("Elapsed (s): %lf\n",elapsed_s);

}


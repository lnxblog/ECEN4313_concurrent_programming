## BUILDING CONCURRENT DATA STRUCTURES ##

The following concurrent data structures have been implemented
1. SGL stack
2. Treiber stack
3. SGL queue
4. M&S queue

## Tests ##
1. Functionality: This test check correctness of the data structure. LIFO in case of stack and FIFO in case of queue.
2. Mulithreaded access: Multiple threads access the structure, the elements inserted and removed are checked if they match.

## Elimination optimization for stacks ##
For the stack data structures, an optional elimination array is used to improve performance. If a thread fails to push or pop from the stack, finds a random position on the elimination array. If the position is not empty and already held by another thread with opposite operation, then both operations can combine and complete each other.

### Performance stats ###

 Each thread performs 100000 operations
 
 #### Stack ####
 
| Stack            | Runtime(s)|  L1 cache hit (%)|	Branch Pred Hit Rate (%)|	Threads (#)       |
|------------------|-----------|------------------|-------------------------|---------------------|
| SGL              | 0.022     |    99.54         |    99.70                |    1                |
| SGL w/o elim     | 0.497     |    98.02         |    99.08                |    4                |
| SGL w/o elim     | 1.833     |   97.48          |    99.24                |    8                |
| SGL w/ elim      | 0.105     |   99.61          |    99.68                |    4                |
| SGL w/ elim      | 0.208     |   99.37          |    99.58                |    8                |
| Treiber          | 0.022     |   99.21          |    99.90                |    1                |
| Treiber w/o elim | 0.201     |   98.94          |    98.75                |    4                |
| Treiber w/o elim | 0.514     |   98.89          |    98.55                |    8                |
| Treiber w/ elim  | 0.095     |   99.59          |    99.67                |    4                |
| Treiber w/ elim  | 0.196     |   99.42          |    99.59                |    8                |

#### Queue ####
| Queue            | Runtime(s)|  L1 cache hit (%)|	Branch Pred Hit Rate (%)|	Threads (#)       |
|------------------|-----------|------------------|-------------------------|---------------------|
| SGL              | 0.109     |    99.40         |    99.46                |    1                |
| M&S Queue        | 0.194     |    98.85         |    98.57                |    4                |


## Code organiztion ##
The Makefile creates three binaries
1. stack_test: main.cpp stack.cpp
2. queue_test: main_q.cpp queue.cpp
3. counter: counter.cpp lifo.cpp

### stack.cpp ###
Contains classes defining SGL stack and Treiber stack. Both stacks use elimination array for optimization.

### queue.cpp ###
Contains classes defining SGL queue and M&S queue.

### main.cpp ###
Contains methods to test the stack structure chosen by user arguments.

### main.cpp ###
Contains methods to test the queue structure chosen by user arguments.

### lifo.cpp ###
Contains TAS lock, MCS lock and MCS LIFO lock class definitions

### counter.cpp ###
Tests the locks defined in lifo.cpp by incrementing a counter with multiple threads chosen based on user arguments.

## Compilation ##

```
make all
```

## Execution ##
```
Stack
Usage: stack_test [--name] [-t num_threads] [--stack=<sgl,treiber>] [-d disable elimination]

Queue
Usage: queue_test [--name] [-t num_threads] [--queue=<sgl,ms>]

Counter
Usage: counter [--name] [-t NUM_THREADS] [-i NUM_ITERATIONS] [--lock=<tas,lifo,mcs>] [-o out.txt]
```
## Extra Credit ##

### Implementation of LIFO MCS lock ###
The MCS lock is modified to hand the lock to threads in LIFO manner. 

LOCK: Each thread tries to set itself as top using CAS. If previously top was NULL then it means the thread has the lock. Else the thread waits on internal atomic state variable to be set by the lock holder.

UNLOCK: 
The lock holder tries to set top to its next node using CAS. If it fails then it means more threads have arrived and top has changed. The lock holder removes itself from the stack by locating its previous node and setting its next to self's next node. The result from the failed CAS holds pointer to the new top. Lock holder sets the new top turn to true.

If the CAS was successful then it provides the lock to the next node following it if not NULL.



### Counter ###
Counter incremented for 100000 iterations by 4 threads each

Performance stats for locking primitives

| Lock             | Runtime(s)|	L1 cache hit (%)|	Branch Pred Hit Rate (%)|	Page Fault Count (#)|
|------------------|-----------|------------------|-------------------------|---------------------|
| Pthread          | 0.021     |    98.34         |    99.16                |  152                |
| Test-and-set     | 0.156     |    49.40         |    94.10                |  150                |
| MCS              | 0.126     |   99.33          |    99.85                |  148                |
| MCS LIFO         | 0.186     |   99.59          |    99.85                |  148                |




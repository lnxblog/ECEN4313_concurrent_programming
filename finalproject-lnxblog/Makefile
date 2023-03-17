all: stack_test queue_test counter

stack_test: main.cpp stack.cpp
	g++-11 -o stack_test main.cpp stack.cpp -g -pthread -std=c++2a 

queue_test: main_q.cpp queue.cpp
	g++-11 -o queue_test main_q.cpp queue.cpp -g -pthread -std=c++2a 

counter: counter.cpp lifo.cpp
	g++-11 -o counter counter.cpp lifo.cpp -g -pthread -std=c++2a 
clean: 
	rm stack_test queue_test counter
  
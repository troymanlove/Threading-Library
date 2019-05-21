
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include "OSPA3.h"

/*
 * Global variables
 */
struct sigaction sigact;

int firstThread = 0;




/**
 * Struct for a circular queue

 */
/*
typedef struct thread{
    void* next;
    void* prev;
    ucontext_t context;
    int id;
    
}thread;

typedef struct CQ{
    thread* head;
    thread* current;
    thread* tail;
    int  consistent;
    int size;
    int init;
    int id;
   
}CQ;

typedef struct Node{
    int id;
    NodePtr * next;
}Node,NodePtr;
typedef struct mutualExclusion{
    int id;
    int locked;
    NodePtr* first;
    NodePtr* iterator;
    
}mutualExclusion;*/

mutualExclusion* mutex;
CQ* circularQueue;

void startTimer(int interval) {
    struct itimerval timer;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    timer.it_value.tv_sec = interval;
    timer.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, 0);
}

/**
 * signal handler
 * receives SIGALRM and then if queue is consistent, swaps context. 
 */
static void signalHandler() {
    //    write(1,"alarm caught\n",sizeof("alarm caught\n"));
    //write(1,circularQueue->consistent,sizeof(int));

    if (circularQueue->consistent == 1) {
        //queue is not consistent so just reset timer and wait
        startTimer(1);
    } else {
        if (circularQueue->size > 1) {
            thread* temp = circularQueue->current;
            circularQueue->current = circularQueue->current->next;
            write(1, "swapping from handler\n", sizeof ("swapping from handler\n"));
            swapcontext(&temp->context, &circularQueue->current->context);
        }
    }
}

/**
 * thread_create()
 * Create a new struct and malloc it
 * use the getcontext() to create the context instance
 * use makecontext() to set this struct's thread value to be 
 * add this struct to the circular queue
 * then call thread_yield() to yield the remainder of its time.
 * 
 * @param context? by use of make context
 * @param function to attach the thread to
 * @param the struct pointer
 * @return void or -1 on fail
 */
void thread_create(thread* t1, void (*func)(void*), void * arg) {

    //the circularQueue has not been intilized yet

    if (firstThread != 1) {
        printf("initilizing everything\n");
        circularQueue = malloc(sizeof (CQ));
        circularQueue->head = NULL;
        sigact.sa_flags = SA_SIGINFO;
        sigact.sa_handler = &signalHandler;
        sigaction(SIGALRM, &sigact, NULL);

        thread* primaryThread = (thread*) malloc(sizeof (thread));

        primaryThread->next = NULL;
        primaryThread->prev = NULL;
        circularQueue->current = primaryThread;
        circularQueue->head = primaryThread;
        circularQueue->tail = primaryThread;
        circularQueue->consistent = 0;
        circularQueue->size = 1;
        circularQueue->init = 1;
        circularQueue->id = 0;
        firstThread = 1;
    }
    //16384 is apparently for addressing?
    //found it online and it works

    char* funcStack = (char*) malloc(16384);
    t1->context.uc_stack.ss_sp = funcStack;
    t1->context.uc_stack.ss_size = 16384;
    t1->context.uc_stack.ss_flags = 0;
    t1->context.uc_link = 0;


    getcontext(&t1->context);
    makecontext(&t1->context, (void(*)(void)) func, 1, arg);
    circularQueue->consistent = 1;
    t1->next = NULL;
    t1->prev = NULL;
    t1->id = circularQueue->id;
    circularQueue->id++;
    circularQueue->size++;
    //insert t1 into queue

    //empty CQ
    if (circularQueue->head == NULL) {
        circularQueue->head = t1;
        circularQueue->tail = t1;
        t1->next = t1;
        t1->prev = t1;

    }        //attach to end of CQ
    else {
        circularQueue->tail->next = t1;
        t1->prev = circularQueue->tail;
        t1->next = circularQueue->head;
        circularQueue->head->prev = t1;
        circularQueue->tail = t1;
    }
    circularQueue->consistent = 0;
    startTimer(1);
    printf("threadid: %d created\n", t1->id);


    //return t1;

}

/**
 * thread_yield()
 * this should have swap context(cq[i],cq[i+1]) then place the current context
 * onto the end of the circular queue
 * @param pass in the thread struct pointer
 * 
 * @return void
 */
void thread_yield() {
    if (circularQueue->consistent == 0) {
        thread* temp = circularQueue->current;
        circularQueue->current = circularQueue->current->next;
        startTimer(1);
        if (circularQueue->current != temp)//checks to see there are more than 1 threads
        {
            printf("Yielding from %d to %d\n", temp->id, circularQueue->current->id);
            swapcontext(&temp->context, &circularQueue->current->context);
        }
    } else {
        printf("unable to yield as queue is inconsistent\n");
    }
}

/**
 * thread_exit()
 * Remove this struct from the circular queue
 * free this struct
 * use set context(cq[i],cq[i+1]) to go to the next item in the circular
 * queue. 
 * @param struct pointer
 * @return 
 */
void thread_exit() {
    circularQueue->consistent = 1;
    thread* temp1 = circularQueue->current->prev;
    thread* temp2 = circularQueue->current->next;
    temp1->next = circularQueue->current->next;
    temp2->prev = circularQueue->current->prev;

    //check to see if it is the only thread left
    if (circularQueue->current == circularQueue->current->next) {
        //finished = 1;
        exit(0);
    }
    //check if head or tail
    if (circularQueue->current == circularQueue->head) {
        circularQueue->head = circularQueue->current->next;
    }
    if (circularQueue->current == circularQueue->tail) {
        circularQueue->tail = circularQueue->current->prev;
    }
    printf("Exiting thread %d\n", circularQueue->current->id);
    circularQueue->size--;
    circularQueue->current = circularQueue->current->next;
    circularQueue->consistent = 0;
    setcontext(&circularQueue->current->context);

}

void lock(mutualExclusion* mutex) {
    printf("entering lock with thread %d\n", circularQueue->current->id);
    printf("mutexid: %d     locked: %d\n",mutex->id,mutex->locked);
    if (mutex->id == circularQueue->current->id) {
        return;
    }
    if (mutex->locked == 1) {
        //already locked so add this thread to the mutex queue
        if (mutex->first != NULL) {
            mutex->iterator = mutex->first;
            //there exists something in queue so add to end of queue
            while (mutex->iterator->next != NULL) {
                //iter = iter->next;
                mutex->iterator = mutex->iterator->next;
            }
            NodePtr* n = malloc(sizeof (NodePtr));

            n->id = circularQueue->current->id;
            n->next = NULL;
            mutex->iterator->next = n;

        } else {
            struct Node* node = malloc(sizeof (NodePtr));
            node->id = circularQueue->current->id;
            node->next = NULL;
            mutex->first = node;
            mutex->iterator = node;
            mutex->first->id = circularQueue->current->id;
        }
        printf("Thread %d is waiting in mutex queue\n", circularQueue->current->id);
        // printf("mutex owner is %d, current thread is %d\n",mutex->id,circularQueue->current->id);
        while (mutex->id != circularQueue->current->id) {
            sleep(1);
            printf("mutex owner is %d, current thread is %d\n", mutex->id, circularQueue->current->id);
            thread_yield();
        }

    } else {
        mutex->locked = 1;
        mutex->id = circularQueue->current->id;
    }
    printf("Thread %d is now the mutex owner\n", circularQueue->current->id);
}

void unlock(mutualExclusion* mutex) {
    printf("entering unlock with thread %d\n", circularQueue->current->id);
    //if owner
    if (mutex->id == circularQueue->current->id) {
        //if queue is not empty
        if (mutex->first != NULL) {
            mutex->id = mutex->first->id;
            if (mutex->first->next != NULL) {
                mutex->first = mutex->first->next;
            } else {
                mutex->first = NULL;
            }
        } else {
            mutex->locked = 0;
            mutex->id = -1;
            mutex->iterator = NULL;
            mutex->first = NULL;
        }
        printf("mutex is now unlocked\n");
    } else {
        printf("Not the owner, cannot unlock\n");
    }
}

int trylock(mutualExclusion* mutex) {
    if (mutex->locked == 1) {
        printf("mutex  is locked. From trylock\n");
        return 1;
    }
    //unlocked
    printf("mutex is unlocked. From trylock\n");
    return 0;
}

void mutex_init(mutualExclusion *mutex) {

    //mutex = malloc(sizeof (mutualExclusion));
    mutex->locked = 0;
    mutex->id = -1;
    mutex->first = NULL;
    mutex->iterator = NULL;



}



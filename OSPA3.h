/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OSPA3.h
 * Author: sickn
 *
 * Created on April 30, 2019, 10:29 AM
 */

#ifndef OSPA3_H
#define OSPA3_H

#include <ucontext.h>

typedef struct Node{
    int id;
    struct Node * next;
}NodePtr;

typedef struct mutualExclusion{
    int id;
    int locked;
    struct Node* first;
    struct Node* iterator;
    
}mutualExclusion;
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



void thread_create(thread* t1,void (*func)(void*),void * arg);
void thread_yield();
void thread_exit();
void lock();
int trylock();
void unlock();
void mutex_init(mutualExclusion* mutex);

#endif
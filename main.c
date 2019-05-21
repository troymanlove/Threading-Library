/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: sickn
 *
 * Created on May 1, 2019, 1:51 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "OSPA3.h"

/*
 * 
 */
int data = 0;
int finished =0;

void workerThread() {
    int i;
    printf("data value when entering is: %d\n", data);
    for (i = 0; i < 3; i++) {
        int temp = data;
        temp++;
        thread_yield();
        data = temp;
        printf("data value is: %d\n", data);

    }
    printf("data value when leaving is: %d\n", data);
    finished++;
    thread_exit();
}
int mutexData = 0;
int finishedMutex=0;
void mutexTest(mutualExclusion *mutex) {
    printf("data value when entering mutex test: %d\n", mutexData);
    int i;
    for (i = 0; i < 3; i++) {
        lock(mutex);
        int temp = mutexData;
        temp++;
        mutexData = temp;
        thread_yield();
        unlock(mutex);

    }
    printf("data value when exiting mutex test: %d\n", mutexData);
    finishedMutex++;
    thread_exit();
}

int main(int argc, char** argv) {

    //circularQueue = (malloc(sizeof(CQ)));
    //mutex = malloc(sizeof(mutualExclusion));
    //circularQueue->init =0;
    mutualExclusion* mutex;
    mutex= malloc(sizeof(mutualExclusion));
    thread t1;
    thread t2;
    thread t3;
    thread t4;
    mutex_init(mutex);
    thread_create(&t1,workerThread,0);
    thread_create(&t2,workerThread,0);
    thread_create(&t3,workerThread,0);
    thread_create(&t4,workerThread,0);
    while(finished <4){
        sleep(1);
        printf("waiting\n");
    }
    /* 
    thread m1;
    thread m2;
    thread m3;
    thread m4;
    printf("mutex data: %d\n",mutex->locked);
    //mutualExclusion mutex;
    //mutex = malloc(sizeof(mutualExclusion));
    //mutex_init(mutex);
    thread_create(&m1, mutexTest, mutex);
    thread_create(&m2, mutexTest, 1,mutex);
    thread_create(&m3, mutexTest, 1,mutex);
    thread_create(&m4, mutexTest, 1,mutex);
    //printf("size of circularQueue: %d\n", circularQueue->size);
    while (finishedMutex <4) {
        sleep(1);
        printf("waiting\n");
    }*/
    printf("All threads have completed. Now exiting\n");

    return (EXIT_SUCCESS);
}


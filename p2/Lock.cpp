#include "Lock.h"
#include "uthread_private.h"

#define BUSY 0
#define FREE 1
// TODO


Lock::Lock() {
    lock_flag = FREE;  // initialze the lock
    //static std::queue<TCB*> waiting_queue;
}

void Lock::lock() {
    disableInterrupts();
    // attempt to acqurie
    if (lock_flag == BUSY) {
        running->setState(BLOCK);
        waiting_queue.push(running);
        switchThreads();
    } else {
        lock_flag = BUSY;
    }
    enableInterrupts();
}

void Lock::unlock() {
    disableInterrupts();
    if (!waiting_queue.empty()) {
        TCB* next = waiting_queue.front();  // get the thread from waiting list
        waiting_queue.pop();
        next->setState(READY);
        addToReady(next);   // optional?
        
        //switchToThread(next); // wake up ??
    } else {
        lock_flag = FREE;
    }
    
    if (!signal_queue.empty())//switch to the signaling thread
	{
		TCB *tcb = signal_queue.front();
		signal_queue.pop();
		switchToThread(tcb);
	}
    
    enableInterrupts();
}

void Lock::_unlock() {
    /*if (!waiting_queue.empty()) {
        running->setState(BLOCK);
        waiting_queue.push(running); 
    }*/
    lock_flag = FREE;
}

void Lock::_signal(TCB *tcb) {
    /*if (tcb != NULL) {
        sig = tcb;
    }*/
    while(lock_flag==BUSY);
    signal_queue.push(running);//enters signal queue
    switchToThread(tcb);
}


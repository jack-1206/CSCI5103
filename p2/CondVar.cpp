#include "CondVar.h"
#include "uthread_private.h"

// TODO
CondVar::CondVar() {
    signaled = 0;
    broadcasted	= 0;
}

void CondVar::wait(Lock &lock) {
    disableInterrupts();
    lock._unlock();
    /*lock._signal(running);
    lock_obj = &lock;
    switchThreads();*/
    /*while(!signaled || !broadcasted);
    if (signaled){
		signaled = 0;
	}else if (broadcasted){
		broadcasted = 0;
	}*/
	waiting_t=running;
	waiting_q.push(running);
	running.setState(BLOCK);
    enableInterrupts();
}

void CondVar::signal() {
    disableInterrupts();
    /*if (!lock_obj->waiting_queue.empty()) {
        lock_obj->_unlock();
        TCB *target = lock_obj->sig;
        lock_obj->_signal(running);
        switchToThread(target);
    }
    signaled = 1;*/
    _signal(waiting_t);
    enableInterrupts();
}

void CondVar::broadcast() {
	for (int i = 0; i < waiting_q.size(); i++)
	{	
		TCB *tcb = waiting_q.front;
		waiting_q.pop();
		_signal(tcb);
	}
	
}

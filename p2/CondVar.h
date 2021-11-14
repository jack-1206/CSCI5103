#ifndef COND_VAR_H
#define COND_VAR_H

#include "TCB.h"
#include "Lock.h"
#include <queue>

// Synchronization condition variable
// NOTE: Follows Hoare semantics
class CondVar {
public:
  CondVar();

  // Release the lock and block this thread atomically. Thread is woken up when
  // signalled or broadcasted
  void wait(Lock &lock);

  // Following Hoare semantics, if a thread is waiting, atomically transfer
  // control to the waiting thread and have control return to this thread after
  // the woken up thread has released the lock
  void signal();

  // Following Hoare semantics, if thread(s) are waiting, atomically transfer
  // control to each in a row and have control return to this thread after the
  // last woken up thread has released the lock (i.e. threads should run in the
  // following order: queue[head], queue[head+1], ..., queue[tail], calling
  // thread)
  void broadcast();

private:
  //int signaled, broadcasted;
  TCB * waiting_t;
  static std::queue<TCB*> waiting_q;//queue for threads waiting for condVar signal
  // TODO - Add members as needed
  Lock *lock_obj;
  //static std::queue<TCB*> signal_queue; // queue for threads waiting to grab lock
};

#endif // COND_VAR_H

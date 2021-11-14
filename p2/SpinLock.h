#ifndef SPIN_LOCK_H
#define SPIN_LOCK_H

#include <atomic>

// Synchronization spinlock
class SpinLock {
public:
  SpinLock();

  // Acquire the lock. Spin until the lock is acquired if the lock is already
  // held
  void lock();

  // Unlock the lock
  void unlock();

private:
  std::atomic_flag atomic_value = ATOMIC_FLAG_INIT;   // Test-and-Set variable
  int value; //1-free	0-busy
};

#endif // SPIN_LOCK_H

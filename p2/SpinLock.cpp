#include "SpinLock.h"

// TODO
SpinLock::SpinLock() {
	value = 1;
}

void SpinLock::lock() {
	while(atomic_value.test_and_set());
}

void SpinLock::unlock() {
	atomic_value.clear();
	value = 1;
}

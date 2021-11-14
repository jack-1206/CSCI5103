#include "uthread.h"
#include "TCB.h"
#include <cassert>
#include <deque>

using namespace std;

void *f(void *arg){
	static int i=1;
	cout<<"In f "<<i<<endl;
	i++;
	return nullptr;
}

TCB *curr;
// Finished queue entry type
typedef struct finished_queue_entry {
  TCB *tcb;             // Pointer to TCB
  void *result;         // Pointer to thread result (output)
} finished_queue_entry_t;
finished_queue_entry_t temp;

// Join queue entry type
typedef struct join_queue_entry {
  TCB *tcb;             // Pointer to TCB
  int waiting_for_tid;  // TID this thread is waiting on
} join_queue_entry_t;
join_queue_entry_t join;

// You will need to maintain structures to track the state of threads
// - uthread library functions refer to threads by their TID so you will want
//   to be able to access a TCB given a thread ID
// - Threads move between different states in their lifetime (READY, BLOCK,
//   FINISH). You will want to maintain separate "queues" (doesn't have to
//   be that data structure) to move TCBs between different thread queues.
//   Starter code for a ready queue is provided to you
// - Separate join and finished "queues" can also help when supporting joining.
//   Example join and finished queue entry types are provided above

// Queues
static deque<TCB*> ready_queue;
static deque<finished_queue_entry_t> finished_queue;
//static deque<TCB*> finished_queue;

TCB* arr[MAX_THREAD_NUM];
// Interrupt Management --------------------------------------------------------

// Start a countdown timer to fire an interrupt
/*static void startInterruptTimer()
{
        // TODO
}

// Block signals from firing timer interrupt
static void disableInterrupts()
{
        // TODO
}

// Unblock signals to re-enable timer interrupt
static void enableInterrupts()
{
        // TODO
}
*/

// Queue Management ------------------------------------------------------------

// Add TCB to the back of the ready queue
void addToReadyQueue(TCB *tcb)
{
        ready_queue.push_back(tcb);
}

void addToFinishedQueue(finished_queue_entry_t tcb)
{
        finished_queue.push_back(tcb);
}
// Removes and returns the first TCB on the ready queue
// NOTE: Assumes at least one thread on the ready queue
TCB* popFromReadyQueue()
{
        assert(!ready_queue.empty());

        TCB *ready_queue_head = ready_queue.front();
        ready_queue.pop_front();
        return ready_queue_head;
}

// Removes the thread specified by the TID provided from the ready queue
// Returns 0 on success, and -1 on failure (thread not in ready queue)
int removeFromReadyQueue(int tid)
{
        for (deque<TCB*>::iterator iter = ready_queue.begin(); iter != ready_queue.end(); ++iter)
        {
                if (tid == (*iter)->getId())
                {
                        ready_queue.erase(iter);
                        return 0;
                }
        }

        // Thread not found
        return -1;
}

int findFromReadyQueue(int tid)
{
        for (deque<TCB*>::iterator iter = ready_queue.begin(); iter != ready_queue.end(); ++iter)
        {
                if (tid == (*iter)->getId())
                {
                        //ready_queue.erase(iter);
                        return 0;
                }
        }

        // Thread not found
        return -1;
}

// Helper functions ------------------------------------------------------------

// Switch to the next ready thread
static void switchThreads()
{
        // TODO
	volatile int flag = 0;
	if (!ready_queue.size())
	{
		return;
	}
	TCB *tcb = popFromReadyQueue();
	int ret_val = curr->saveContext();
	if (ret_val < 0)
	{
		perror("getContext error");
		exit(1);
	}
	if(flag == 1){
		return;
	}
	flag = 1;
	tcb->setState(RUNNING);
	curr = tcb;
	tcb->loadContext();
}


// Library functions -----------------------------------------------------------

// The function comments provide an (incomplete) summary of what each library
// function must do
// Starting point for thread. Calls top-level thread function
void stub(void *(*start_routine)(void *), void *arg)
{
        // TODO
    temp.result = start_routine(arg);
    uthread_exit(curr);
}

int uthread_init(int quantum_usecs)
{
        // Initialize any data structures
    struct sigaction act;
    struct itimerval timer;
    act.sa_handler = (sighandler_t)switchThreads;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
        // Setup timer interrupt and handler
    if (sigaction(SIGVTALRM, &act, nullptr) < 0)
	{
		perror("sigaction error");
		return -1;
	}
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = quantum_usecs;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = quantum_usecs;
    if (setitimer(ITIMER_VIRTUAL, &timer, nullptr) == -1)
	{
		perror("setitimer error");
		return -1;
	}
        // Create a thread for the caller (main) thread
	TCB *main = new TCB(0, nullptr, nullptr, RUNNING);
	curr = main;
	arr[0] = main;
	main->saveContext();
	return 1;
}

int uthread_create(void* (*start_routine)(void*), void* arg)
{
	// Create a new thread and add it to the ready queue	
	static int id = 1;
	if(id <= MAX_THREAD_NUM){
		curr->disableInt();
		TCB *tcb = new TCB(id, start_routine, arg, READY);
		addToReadyQueue(tcb);
		arr[id] = tcb;
		id++;
		curr->enableInt();
		return id-1;
	}else{
		return -1;
	}
}

/*int uthread_join(int tid, void **retval)
{
        // If the thread specified by tid is already terminated, just return
    if (finished[tid-1] == tid)
	{
		return 0;
	}
        // If the thread specified by tid is still running, block until it terminates
    if (findFromReadyQueue(int tid) == -1)
	{
		//
	}
        // Set *retval to be the result of thread if retval != nullptr
}*/

int uthread_yield(void)
{
        // TODO
    if (ready_queue.empty()){
		return -1;
	}
    temp.tcb = curr;
    temp.result = nullptr;
    addToFinishedQueue(temp);
    switchThreads();
    return 0;
}

void uthread_exit(void *retval)
{
        // If this is the main thread, exit the program
    if (uthread_self() == 0)
	{
		exit(0);
	}
        // Move any threads joined on this thread back to the ready queue
        // Move this thread to the finished queue
    temp.tcb = curr;
    temp.result = retval;
    addToFinishedQueue(temp);
    switchThreads();
}

/*int uthread_suspend(int tid)
{
        // Move the thread specified by tid from whatever state it is
        // in to the block queue
}

int uthread_resume(int tid)
{
        // Move the thread specified by tid back to the ready queue
}*/

int uthread_self()
{
        // TODO
	return curr->getId();
}

/*int uthread_get_total_quantums()
{
        // TODO
}

int uthread_get_quantums(int tid)
{
        // TODO
}
*/


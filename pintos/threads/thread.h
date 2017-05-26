#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
#include "threads/fpr_arith.h"
#include "threads/synch.h"
#include <kernel/hash.h>
#include "filesys/filesys.h"


/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */

/* Thread nicesses. */
#define NICE_MIN -20                     /* Lowest niceness. */
#define NICE_DEFAULT 0                   /* Default niceness. */
#define NICE_MAX 20                      /* Highest niceness. */

/* Max filename length for a pintos executable.
 * Comes from the original Pintos code and I made it into a definition. */
#define MAX_THREADNAME_LENGTH 16

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[MAX_THREADNAME_LENGTH];   /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    struct list_elem allelem;           /* List element for all threads list. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. Used either for ready_list or sleeping_list. */

    /* needed for file syscalls*/


#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t * pagedir;                 /* Page directory. */
    int exit_status;                    /* Status passed to exit() */
    struct list children_data;          /* List of children of this thread; stays after children finish. */
    struct semaphore sem_child_loaded; /* Makes this thread block when creating a new child, which unblocks it. */
    bool child_load_error;            /* Used for the child to inform its parent about creation. */
    struct thread* parent;              /* Parent thread */
#endif

    int64_t wakeup_at_tick;

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */

    /* Advance scheduling data */
    int priority;                       /* Priority. */
    int nice;                           /* Niceness value. */
    FPReal recent_cpu;                  /* Recent cpu usage of the thread. */
  };

/* Used for the wait(tid) system call. The parent must keep track of the
 * threads that it has created, so that it can return their correct exit
 * status, even after they've finished. The child_thread item can be
 * removed only after the thread has been waited for.
 *    The alternative for this would be to assume that a thread only waits
 * its last child, which is not part of the specification. */
struct child_thread_data {
    struct thread * thread_ref;
    tid_t tid;
    int exit_status;
    struct semaphore sem_exited;
    struct list_elem elem;
};

struct child_thread_data * thread_get_child_data(struct thread * parent, tid_t child_tid);

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

struct thread * thread_get_by_tid (int tid);

void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);
void thread_yield_on_higher_priority (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *thread_ref, void *aux);
void thread_foreach (thread_action_func *, void *);

int thread_get_priority (void);
void thread_set_priority (int);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

void thread_sleep (int64_t wakeup_at);

bool thread_priority_cmp (const struct list_elem* a,
  const struct list_elem* b,
  void* aux);

#endif /* threads/thread.h */

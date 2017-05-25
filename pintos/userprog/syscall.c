#include "userprog/syscall.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/palloc.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "userprog/process.h"
#include "devices/shutdown.h"

static void syscall_handler (struct intr_frame *);

typedef void (*handler) (struct intr_frame *);
static void syscall_exit (struct intr_frame *);
static void syscall_exec (struct intr_frame *);
static void syscall_wait (struct intr_frame *);
static void syscall_write (struct intr_frame *);
static void syscall_halt (struct intr_frame *);
static void syscall_create (struct intr_frame *f);
static void syscall_remove (struct intr_frame *f);
static void syscall_open (struct intr_frame *f);
static bool check_user_address (void *);

#define SYSCALL_MAX_CODE 19
static handler call[SYSCALL_MAX_CODE + 1];

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");

  /* Any syscall not registered here should be NULL (0) in the call array. */
  memset(call, 0, SYSCALL_MAX_CODE + 1);

  /* Check file lib/syscall-nr.h for all the syscall codes and file
   * lib/user/syscall.c for a short explanation of each system call. */
  call[SYS_EXIT]  = syscall_exit;   /* Terminate this process. */
  call[SYS_EXEC]  = syscall_exec;   /* Start another process. */
  call[SYS_WAIT]  = syscall_wait;   /* Wait for a child process to die. */
  call[SYS_WRITE] = syscall_write;  /* Write to a file. */
	call[SYS_HALT] = syscall_halt;	/* Halts pintOS. */
	call[SYS_CREATE] = syscall_create; /* Creates a new file. */
	call[SYS_REMOVE] = syscall_remove; /* Removes a file. */
	call[SYS_OPEN] = syscall_open; /* Opens a file. */
}

static void
syscall_handler (struct intr_frame *f)
{
  int syscall_code = *((int*)f->esp);
  call[syscall_code](f);
}

static void
syscall_exit (struct intr_frame *f)
{
  int *stack = f->esp;
  struct thread* t = thread_current ();
  t->exit_status = *(stack+1);
  thread_get_child_data(t->parent, t->tid)->exit_status = t->exit_status;
  thread_exit ();
}

static void
syscall_create (struct intr_frame *f)
{
  int *stack = f->esp;
	char * name = (char *) *(stack + 1);
	unsigned size = (unsigned) *(stack + 2);

	f->eax = filesys_create (name, size);
}

static void
syscall_open (struct intr_frame *f)
{
  int *stack = f->esp;
	char * name = (char *) *(stack + 1);
  struct file* filepointer = filesys_open(name);


	struct file_descriptor *descriptor = malloc (sizeof (struct file_descriptor));
    if (!descriptor)
      syscall_exit (-1);

			struct thread * current = thread_current();

    descriptor->fd = timer_ticks();
    descriptor->file = filepointer;

    hash_insert (&current->fd_table, &descriptor->h_elem);
  // take filepointer
  // generate a fd (we use timer tick)
  // put fd into a hashtable
  // store in another table the <hash(fd), filepointer>
  // return fd
	f->eax = -1;
}

static void
syscall_remove (struct intr_frame *f)
{
  int *stack = f->esp;
	char * name = (char *) *(stack + 1);

	f->eax = filesys_remove (name);
}

static void
syscall_halt (struct intr_frame *f)
{
  shutdown_power_off();
}

static void
syscall_exec (struct intr_frame * f)
{
  int * stackpointer = f->esp;
  char * command = (char *) *(stackpointer + 1);

  if (check_user_address (command))
    f->eax = process_execute (command);
  else
    f->eax = -1;
}

static void
syscall_wait (struct intr_frame * f)
{
  int * stackpointer = (void *) f->esp;
  tid_t child_tid = *(stackpointer + 1);
  f->eax = process_wait (child_tid);
}

static void
syscall_write (struct intr_frame *f)
{
  int *stack = f->esp;
  ASSERT (*(stack+1) == 1); // fd 1 means stdout (standard output)
  char * buffer = *(stack+2);
  int    length = *(stack+3);
  putbuf (buffer, length);
  f->eax = length;
}

static bool check_user_address (void * ptr) {
  return ptr != NULL && is_user_vaddr (ptr) && pagedir_get_page (thread_current ()->pagedir, ptr);
}

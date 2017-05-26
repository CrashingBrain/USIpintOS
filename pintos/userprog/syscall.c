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
#include "devices/timer.h"
#include "devices/input.h"

static void syscall_handler (struct intr_frame *);

struct semaphore using_fs;
struct hash fd_table;

struct lock filesys_lock;

typedef void (*handler) (struct intr_frame *);
static void syscall_exit (struct intr_frame *);
static void syscall_exec (struct intr_frame *);
static void syscall_wait (struct intr_frame *);
static void syscall_write (struct intr_frame *);
static void syscall_read (struct intr_frame *);
static void syscall_halt (struct intr_frame *);
static void syscall_create (struct intr_frame *f);
static void syscall_remove (struct intr_frame *f);
static void syscall_open (struct intr_frame *f);
static void syscall_close (struct intr_frame *f);
static bool check_user_address (void *);

#define SYSCALL_MAX_CODE 19
static handler call[SYSCALL_MAX_CODE + 1];


//this function returns the fd associated to a certain hash element
unsigned fd_hash_function (const struct hash_elem *e,
												 void *aux UNUSED){
  struct file_descriptor * descriptor =  hash_entry (e, struct file_descriptor, h_elem);
  return descriptor->fd;
}

//this function return the lesser fd of file descriptors
bool fd_less_function (const struct hash_elem *a,
                     const struct hash_elem *b,
                     void *aux UNUSED){
  struct file_descriptor *d_a =  hash_entry (a, struct file_descriptor, h_elem);
  struct file_descriptor *d_b =  hash_entry (b, struct file_descriptor, h_elem);

  return d_a->fd < d_b->fd;
}


void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");

	hash_init(&fd_table,
		&fd_hash_function,
		&fd_less_function,
		NULL);

	sema_init(&using_fs, 1);

  /* Any syscall not registered here should be NULL (0) in the call array. */
  memset(call, 0, SYSCALL_MAX_CODE + 1);

  /* Check file lib/syscall-nr.h for all the syscall codes and file
   * lib/user/syscall.c for a short explanation of each system call. */
  call[SYS_EXIT]  = syscall_exit;   /* Terminate this process. */
  call[SYS_EXEC]  = syscall_exec;   /* Start another process. */
  call[SYS_WAIT]  = syscall_wait;   /* Wait for a child process to die. */
  call[SYS_WRITE] = syscall_write;  /* Write to a file. */
  call[SYS_READ] = syscall_read;    /* Read from a file. */
	call[SYS_HALT] = syscall_halt;	/* Halts pintOS. */
	call[SYS_CREATE] = syscall_create; /* Creates a new file. */
	call[SYS_REMOVE] = syscall_remove; /* Removes a file. */
	call[SYS_OPEN] = syscall_open; /* Opens a file. */
  call[SYS_CLOSE] = syscall_close; /* Closes a file */
}

static void
syscall_handler (struct intr_frame *f)
{
	sema_up(&using_fs);
  int syscall_code = *((int*)f->esp);
  call[syscall_code](f);
	sema_down(&using_fs);
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

  if (check_user_address (name))
  {
    bool result = filesys_create (name, size);
    f->eax = result;
  }
  else
  {
    *(stack+1) = -1;
    syscall_exit(f);
  }
}

static void
syscall_open (struct intr_frame *f)
{
  int *stack = f->esp;
	char * name = (char *) *(stack + 1);

  if (check_user_address(name))
  {
    struct file* filepointer = filesys_open(name);


    struct file_descriptor * desc = malloc (sizeof (struct file_descriptor));
    if (!desc){
      f->eax = -1;
      return;
    }

    desc->fd = timer_ticks();
    desc->file = filepointer;

    struct thread * current = thread_current();

    hash_insert (&fd_table, &desc->h_elem);
    // take filepointer
    // generate a fd (we use timer tick)
    // put fd into a hashtable
    // store in another table the <hash(fd), filepointer>
    // return fd
    f->eax = desc->fd;
  } else {
    *(stack+1) = -1;
    syscall_exit(f);
  }

}

static void
syscall_close (struct intr_frame *f){
  int *stack = f->esp;
  int fd = *(stack+1);

  if (fd > 1) // error if closing stdin or stdout
  {
    // iterate in list of files
    // find for corrensonding fd
    // get filepointer
    // call file_close(filepointer)
    // remove from list/hash
    // return
    // ALSO make syscall_exit call this for every own files (if tests still don't pass)
  }
  return;

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
  int fd = *(stack+1);
  // get buffer at stack+2
  // get size at stack+3
  if(fd == 1){
    char * buffer = *(stack+2);
    int    length = *(stack+3);
    putbuf (buffer, length);
    f->eax = length;
  }
  // else
  // iterate in list of files
  // find for corrensonding fd
  // get filepointer
  // call file_write(fd, buffer, size)
  // store number of bytes wrote in eax
}

static void
syscall_read (struct intr_frame *f){
  int *stack = f->esp;
  int fd = *(stack+1);
	char * to_write = *(stack+2);
	int size = *(stack+3);

	if(fd == 0){
		// getc();
		char c;
		int i=0;
		while (i < size){
			c = input_getc();
			*(to_write+i) = c;
			i++;
		}
		f->eax = i;
		return;
	} else if(fd == 1){
		*(stack+1) = -1;
    syscall_exit(f);
	} else {
		struct file_descriptor to_find;
		to_find.fd = fd;
		struct hash_elem * e = hash_find(&fd_table, &to_find.h_elem);
		struct file_descriptor * found =  hash_entry (e, struct file_descriptor, h_elem);
		struct file * to_read = found->file;

		int bytes_read = file_read(to_read, to_write, size);
		f->eax = bytes_read;
	}

  // if fd == 0
  // use getc() to read from stdin
  // else
  // iterate in list of files
  // find for corrensonding fd
  // get filepointer
  // call file_read(fd, buffer, size)
  // store number of bytes read in eax

}

static bool check_user_address (void * ptr) {
  return ptr != NULL && is_user_vaddr (ptr) && pagedir_get_page (thread_current ()->pagedir, ptr);
}

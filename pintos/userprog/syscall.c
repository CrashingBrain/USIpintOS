#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);
void exit (int status);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
	//TODO here handle printf and exit
  printf ("system call!\n");
  int sysnumber = * (int *) f->esp;
  int * esp = f->esp;
  printf("%d\n", sysnumber);
  switch(sysnumber){
  	case SYS_WRITE: // second to do
  		{
  			if (*(esp+1) == 1) putbuf(*(esp+2), *(esp+3));
  			break;
  		}
  	case SYS_EXIT: //first to do
  		{
  			exit(* (int *) esp + 1 );
  			break;
  		}
  	default:
  		break;
  }
}


void exit (int status)
{
  struct thread *cur = thread_current();
  // if (thread_alive(cur->parent))
  //   {
  //     cur->cp->status = status;
  //   }
  printf ("%s: exit(%d)\n", cur->name, status);
  thread_exit();
}

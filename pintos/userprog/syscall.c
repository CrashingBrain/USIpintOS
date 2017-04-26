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
  int sysnumber = * (int *) f->esp;
  int * esp = f->esp;
  // printf("%d\n", sysnumber);
  switch(sysnumber){
  	case SYS_WRITE: // second to do
  		{
        // printf("fd : %d buff: %s size: %d\n", *(esp+1), *(esp+2), * (char*) (esp+3));
  			if (*(esp+1) == 1) {
          // printf("\nprintf: %s\nputbuf: ", *(esp+2));
          putbuf(*(esp+2), *(esp+3));
        }
  			break;
  		}
  	case SYS_EXIT: //first to do
  		{
        // printf("status: %d %d\n", * (int *) esp, * (int *)(esp + 1));
  			exit(* (int *)(esp + 1));
  			break;
  		}
  	default:
  		break;
  }
}


void exit (int status)
{
  struct thread *cur = thread_current();

  printf ("%s: exit(%d)\n", strtok_r(cur->name, " "), status);
  thread_exit();
}

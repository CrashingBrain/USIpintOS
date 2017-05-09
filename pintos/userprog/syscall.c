#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include <user/syscall.h>

static void syscall_handler (struct intr_frame *);
void exit (int status);
pid_t exec (const char *cmd_line);
int wait (pid_t pid);


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
    case SYS_WAIT:
      {
        // call wait() here
        f->eax = wait(* (int *)(esp + 1));
        break;
      } 
    case SYS_EXEC:
      {
        f->eax = exec((char*) *(esp+1));
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
  cur->exitstatus = status;
  cur->terminated = true;
  thread_exit();
}

pid_t exec (const char *cmd_line)
{
  // check validity of pointer before delivering it to critical code, maybe?
  pid_t pid = process_execute(cmd_line);
  //TODO
  // Check if creation was successful otherwise return -1

  return pid;
}

int wait (pid_t pid){

  struct thread * child = thread_get_children_by_tid(pid);
  if(child == NULL) {
      // printf("%d CAZZO SEI QUI?\n", pid);
    return -1;
  } else if(child->status == THREAD_DYING){
      // printf("CAZZO SEI QUÀ?\n");

    return -1;
  } else {
    while(!child->terminated){

    }
    return child->status;
  }


  // printf("CAZZO status exitstatus %d\n", child->exitstatus);
  return child->exitstatus;
}

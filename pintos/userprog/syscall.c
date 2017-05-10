#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
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
        bad_ptr((const void *)(esp + 1));
  			exit(* (int *)(esp + 1));
  			break;
  		}
    case SYS_WAIT:
      {
        // call wait() here
        bad_ptr((int *)(esp + 1));
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

void bad_ptr (const void *ptr)
{
  if (!is_user_vaddr(ptr) || ptr < ((void *) 0x08048000) || !lookup_page(thread_current()->pagedir, ptr, false))
    {
      exit(-1);
    }
}

void exit (int status)
{
  struct thread *cur = thread_current();

  printf ("%s: exit(%d)\n", strtok_r(cur->name, " "), status);
  cur->exitstatus = status;
  cur->terminated = true;

	// struct thread * parent = thread_get_by_tid(cur->parentId);
	// if(parent != NULL || parent != 0){
	// 	sema_up(&parent->exec_sema);
	// }
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
	return process_wait(pid);
}

#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdbool.h>
#include <kernel/hash.h>


void syscall_init (void);

struct file_descriptor {
	int fd;
	struct file * file;
	struct hash_elem h_elem;
};

unsigned fd_hash_function (const struct hash_elem *e, void *aux);

bool fd_less_function (const struct hash_elem *a, const struct hash_elem *b, void *aux);

#endif /* userprog/syscall.h */

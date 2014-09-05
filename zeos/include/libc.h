/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

extern int errno;

void perror();
void itoa(int a, char *b);
int strlen(char *a);
int write(int fd, char *buffer, int size);

int getpid();
int fork();
void exit();
int get_stats(int pid, struct stats *st);
int gettime();
int clone(void (*func) (void), void *stack);
int read(int fd, char *buff, int count);
void *srbk(int increment);

/*clone polithics*/
int sem_init(int n_sem, unsigned int value);
int sem_wait(int n_sem);
int sem_signal(int n_sem);
int sem_destroy(int n_sem);


#endif  /* __LIBC_H__ */

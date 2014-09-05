/*
 * libc.c 
 */

#include <libc.h>
#include <errno.h>
#include <types.h>

int errno;

void perror()
{
	switch(errno) {
		case ENOSYS:
			write(1, "\nNot implemented\n", 13);
		  break;
		case EFAULT:
			write(1, "\nBad address\n", 13);
			break;
		case EINVAL:
			write(1, "\nInvalid argument\n", 18);
			break;
		case ESRCH:
			write(1, "\nProcess id doesn't exist\n", 26);
			break;
		case EBADF:
			write(1, "\nBad file permission\n", 21);
		default:
			write(1, "\nUnknown error\n", 15);
			break;
	}
}

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

int write (int fd, char *buffer, int size) {
	int res;
	__asm__ __volatile__ (
		"pushl %%ebx;"
		"movl 8(%%ebp), %%ebx;"
		"movl 12(%%ebp), %%ecx;"
		"movl 16(%%ebp), %%edx;"
		"movl $4, %%eax;"
		"int $0x80;"
		"movl %%eax, %0;"
		"popl %%ebx;"
		: "=g" (res)

	);
	/*retorna -1 a l'usuari si error*/
	if (res < 0) {
		errno = -res;
		res = -1;
	}
	return res; 						
}

int gettime() {
	int res;
	__asm__ __volatile__ (
		"movl $10, %%eax;"
		"int $0x80;"
		"movl %%eax, %0;"
		: "=g" (res)
	);
	return res;		
}

int getpid() {
	int pid;
	__asm__ __volatile__ (
		"movl $20, %%eax;"
		"int $0x80;"
		"movl %%eax, %0;"
		: "=g" (pid)
	);
	return pid;
}

void exit() 
{
	__asm__ __volatile__ (
		"movl $1, %eax;"
		"int $0x80;"
	);
}

int fork() 
{
	int res;
	__asm__ __volatile__ (
		"movl $2, %%eax;"
		"int $0x80;"
		"movl %%eax, %0;"
		: "=g" (res)
	);
	if (res < 0) {
		errno = -res;
		res = -1;
	}
	return res;
}

int get_stats(int pid, struct stats *st)
{
	int res;
	__asm__ __volatile__(
		"movl $35, %%eax;"
		"pushl %%ebx;" //registre a guardar si es modifica
		"movl %1, %%ebx;" //passem 1r parametre
		"movl %2, %%ecx;" //passem 2n parametre
		"int $0x80;"		//crida a sistema
		"movl %%eax, %0;" //recuperem valor de retorn
		"popl %%ebx;"
		:"=g" (res)
		:"g"(pid), "g"(st)
	);
	/*retorna -1 a l'usuari si error*/
	if (res < 0) {
		errno = -res;
		res = -1;
	}
	return res;	
}

int clone(void (*func) (void), void *stack)
{
	int res;
	__asm__ __volatile__(
		"movl $19, %%eax;"
		"pushl %%ebx;"
		"movl %1, %%ebx;"
		"movl %2, %%ecx;"
		"int $0x80;"
		"movl %%eax, %0;"
		"popl %%ebx;"
		:"=g" (res)
		:"g" (func), "g"(stack)
	);
	if (res < 0) {
		errno = -res;
		res = -1;
	}
	return res;
}
		
int sem_init(int n_sem, unsigned int value) {
	int res;
	__asm__ __volatile__(
		"movl $21, %%eax;"
		"pushl %%ebx;"
		"movl %1, %%ebx;"
		"movl %2, %%ecx;"
		"int $0x80;"
		"movl %%eax, %0;"
		"popl %%ebx;"
		:"=g" (res)
		:"g"(n_sem), "g"(value)
	);
	if (res < 0) {
		errno = -res;
		res = -1;
	}
	return res;
}	

int sem_wait(int n_sem) {
	int res;
	__asm__ __volatile__(
		"movl $22, %%eax;"
		"pushl %%ebx;"
		"movl %1, %%ebx;"
		"int $0x80;"
		"movl %%eax, %0;"
		"popl %%ebx;"
		:"=g" (res)
		:"g"(n_sem)
	);
	if (res < 0) {
		errno = -res;
		res = -1;
	}
	return res;
}	
		
int sem_signal(int n_sem) {
	int res;
	__asm__ __volatile__(
		"movl $23, %%eax;"
		"pushl %%ebx;"
		"movl %1, %%ebx;"
		"int $0x80;"
		"movl %%eax, %0;"
		"popl %%ebx;"
		:"=g" (res)
		:"g"(n_sem)
	);
	if (res < 0) {
		errno = -res;
		res = -1;
	}
	return res;
}		
		
int sem_destroy(int n_sem) {
	int res;
	__asm__ __volatile__(
		"movl $24, %%eax;"
		"pushl %%ebx;"
		"movl %1, %%ebx;"
		"int $0x80;"
		"movl %%eax, %0;"
		"popl %%ebx;"
		:"=g" (res)
		:"g"(n_sem)
	);
	if (res < 0) {
		errno = -res;
		res = -1;
	}
	return res;
}

int read(int fd, char *buff, int count) {
	int res;
	__asm__ __volatile__(
		"movl $3, %%eax;"
		"pushl %%ebx;"
		"movl %1, %%ebx;"
		"movl %2, %%ecx;"
		"movl %3, %%edx;"
		"int $0x80;"
		"movl %%eax, %0;"
		"popl %%ebx;"
		:"=g" (res)
		:"g"(fd), "g"(buff), "g"(count)
	);
	if (res < 0) {
		errno = -res;
		res = -1;
	}
	return res;
}

void *sbrk(int increment) {
	int pointer;
	__asm__ __volatile__(
		"movl $5, %%eax;"
		"pushl %%ebx;"
		"movl %1, %%ebx;"
		"int $0x80;"
		"movl %%eax, %0;"
		"popl %%ebx;"
		:"=g" (pointer)
		:"g" (increment)
	);
	if (pointer < 0) {
		errno = -pointer;
		pointer = -1;
	}
	return pointer;
}




#include <libc.h>
#include <io.h>
void runjp();
void runjp_rank(int first, int last);

char buff[128];

int pid;

long inner(long n) {
        int i;
        long suma;
        suma = 0;
        for (i=0; i<n; i++) suma = suma + i;
        return suma;
}

long outer(long n) {
        int i;
        long acum;
        acum = 0;
        for (i=0; i<n; i++) acum = acum + inner(i);
        return acum;
}

void make_writes() {
	int k;
	k = write(1, "WRITES\n", 7);
	if (k != 7) perror();
	k = write(1, "VATER\n", 6);
	if (k!= 6) perror();
	k = write(1, "\n\n", 2);
	if (k != 2) perror();
	k = write(1, "HOLA?\n", 6);
	if (k != 6) perror();
	exit();
}

void print_time(int num) {
	int i = 0;
	int k;
	char time[20];
	int m = 1;
	while (i < num) {
		k = gettime();
		itoa(k, time);
		if (k == m) {
			k = write(1, time, strlen(time));
			if (k != strlen(time)) perror();
			k = write(1, ", ", 2);
			if (k != 2) perror();
			m += 2^i;
			i++;
		}
	}
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
    runjp();
	//runjp_rank(0,2);
	//runjp_rank(4,7);
	//if (clone(make_writes, buff) > -1) write(1, "YES\n", 4);
	//else write(1, "NO\n", 3); 
	//exit();
	/*int i = 0;
	char c[22];
	char x;
	while (i < 20) {
		//write(1, "\nuser\n", 6);
		int k = read(0, &c,22);
		write(1, &c, 22);
		write(1, "\nsize \n", 7);
		itoa(k, &x);
		write(1, &x, 1);
		i++;
	}*/
  	while(1) {
		
 	}
}

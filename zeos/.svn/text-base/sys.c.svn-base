/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>
#include <system.h>
#include <utils.h>
#include <io.h>
#include <mm.h>
#include <mm_address.h>
#include <sched.h>
#include <errno.h>
#include <interrupt.h>

#define LECTURA 0
#define ESCRIPTURA 1
#define MAX_SIZE 512

int check_fd(int fd, int permissions)
{
  if (fd!=1 && fd != 0) return -EBADF;
  if (fd == 1 && permissions!=ESCRIPTURA) return -EBADF;
  if (fd == 0 && permissions!=LECTURA) return -EBADF;
  return 0;
}

int sys_ni_syscall()
{
	return -ENOSYS;
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
	/*if (error) PID =-1, if (parent) PID=PIDchild, if (child) PID = 0*/
  int PID=-1;
	int i; 
	if (list_empty(&freequeue) == 1) return -ENOMEM;

	/*getting free frames*/
	unsigned int NUM_PAG = ((unsigned int)current()->brk >> 12) - 
		(NUM_PAG_KERNEL + NUM_PAG_CODE);
  	if((unsigned int)current()->brk > (NUM_PAG_KERNEL + 
  		NUM_PAG_CODE + NUM_PAG_DATA)<<12)NUM_PAG++;
	int frames[NUM_PAG];
	for (i = 0; i < NUM_PAG; ++i) {
		frames[i] = alloc_frame();
		if (frames[i] == -1) { //no more frames available
			//unallocating unused space
			for (i; i >= 0; i--) free_frame(frames[i]);
			return -ENOMEM;
		}
	}

	/*getting parent & child structures*/
	union task_union *child = (union task_union *)list_head_to_task_struct(
		list_first(&freequeue) );
	list_del(&child->task.list); //eliminem punter a llista free
	union task_union * parent = (union task_union *)current();

	/*copying inherit system data*/
	copy_data(parent, child, sizeof(union task_union));
	
	/*initializing for copying system data*/
	allocate_DIR(&child->task);
	page_table_entry * parentPT = get_PT(&parent->task);
	page_table_entry * childPT = get_PT(&child->task);
	
	for (i = 0; i < NUM_PAG_CODE; i++) {
		/*copying system code*/
		childPT[PAG_LOG_INIT_CODE_P0+i] = parentPT[PAG_LOG_INIT_CODE_P0+i];
	}

	for (i = 0; i < NUM_PAG; i++) {
		/*copying data*/
		set_ss_pag(parentPT, PAG_LOG_INIT_DATA_P0+NUM_PAG+i, frames[i]);
		set_ss_pag(childPT, PAG_LOG_INIT_DATA_P0+i, frames[i]);
		copy_data((void*)(PAGE_SIZE*(PAG_LOG_INIT_DATA_P0+i)), (void*)(PAGE_SIZE*(
			PAG_LOG_INIT_DATA_P0+NUM_PAG + i)), PAGE_SIZE);
		del_ss_pag(parentPT, PAG_LOG_INIT_DATA_P0+NUM_PAG+i); //deleting copy
	}
	
	set_cr3(get_DIR(&parent->task)); //free of temporally entries-> TLB flush

	/*restoring inherit system data*/
  unsigned int ebp = 0;
  __asm__ __volatile__(
	  "movl %%ebp, %0;"
    : "=g" (ebp)
  );
  int p =  (ebp & 0x0000fff)/4;
  child->stack[p] = (unsigned int)ret_from_fork;
  child->stack[p-1] = (unsigned int)0;
  child->task.kernel_esp = (unsigned int)&(child->stack[p-1]);
	
	/*Initialization of child process*/
	list_add_tail(&child->task.list, &readyqueue);
	PID = child->task.PID = nPID++;
	child->task.remaining = 0;

	/*Initialization of statistics*/
	child->task.quantum = QUANTUM;
	update_stats(&child->task);
	child->task.statistics.system_ticks =get_ticks();
	
	return PID;
}

void sys_exit()
{
	destroy_proc_semaphores(); //destruïm semaphores creats
	//page_table_entry *pt = get_PT(current()); //agafem punter a taula pag
	desallocate_DIR(); //alliberem PTentry i ho indiquem al vector
	current()->PID = -1; //dead process
	update_current_state_rr(&freequeue); //task_struct passa a freequeue
	sched_next_rr(); //next process to be run
}

int sys_write(int fd, char * buffer, int size) 
{
	char aux[MAX_SIZE];
	int written = 0;
	int k = check_fd(fd,ESCRIPTURA);
	if (k != 0) return k;
	if (buffer == NULL) return -EFAULT;
	if (size < 0) return -EINVAL;
	while (size > MAX_SIZE) {
		if (copy_from_user(buffer, aux, MAX_SIZE) != 0) return -EFAULT;
		written += sys_write_console(aux, MAX_SIZE);
		size -= MAX_SIZE;
		buffer += MAX_SIZE;
	}
	if (copy_from_user(buffer, aux, size) != 0) return -EFAULT;
	written += sys_write_console(aux, size);
	return written;
}

int sys_gettime() {
	return zeos_ticks;
}

int ret_from_fork() {
	return 0;
}

int sys_get_stats(int pid, struct stats *st)
{
	int i;
	if (pid<0) return -ESRCH;
	if (st == NULL) return -EFAULT;
	if (! access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT;
	for (i = 0; i < NR_TASKS; ++i) {
		if (task[i].task.PID == pid) {
			copy_to_user(&task[i].task.statistics, st, STATS_SIZE);
			return 0;
		}		
	}
	return -ESRCH;
}

int sys_clone(void (*func) (void), void *stack) {
	/*if (error) PID =-1, if (parent) PID=PIDclone, if (clone) PID = 0*/
  int PID;
	if (list_empty(&freequeue) == 1) return -ENOMEM;
	if (func == NULL || stack == NULL) return -EFAULT;
	if (access_ok(VERIFY_WRITE, func, STATS_SIZE) == 0) return -EFAULT;
	if (access_ok(VERIFY_WRITE, stack, STATS_SIZE) == 0) return -EFAULT;
	union task_union *clone = (union task_union *)list_head_to_task_struct(
		list_first(&freequeue) );
	list_del(&clone->task.list); //eliminem punter a llista free
	union task_union * parent = (union task_union *)current();
	
	/*copying inherit data*/
	copy_data(parent, clone, PAGE_SIZE);

	/*restoring uninherit system data*/
  unsigned int ebp = 0;
  __asm__ __volatile__(
	  "movl %%ebp, %0;"
    : "=g" (ebp)
  );
  int p =  (ebp & 0x0000fff)/4;
  clone->stack[p] = (unsigned int)ret_from_fork;
  clone->stack[p-1] = (unsigned int)0;
  clone->task.kernel_esp = (unsigned int)&(clone->stack[p-1]);

	/*updating clone stack*/
	clone->stack[KERNEL_STACK_SIZE-5] = (unsigned int)func; //eip
	clone->stack[KERNEL_STACK_SIZE-2] = (unsigned int)stack; //esp

	/*Initialization of statistics*/
	clone->task.quantum = QUANTUM;
	update_stats(&clone->task);
	clone->task.statistics.system_ticks =get_ticks();

	/*Initialization of clone process*/
	list_add_tail(&clone->task.list, &readyqueue);
	update_DIR(); //increments de page_entry vector allocDIR
	PID = clone->task.PID = nPID++;
	return PID;
}

int sys_sem_init(int n_sem, unsigned int value) {
	if (n_sem < 0 || n_sem > NR_SEM-1) return -EINVAL; //n_sem no valido
	if (semaphores[n_sem].owner != -1) return -EBUSY; //si ya está usado
	semaphores[n_sem].owner = current()->PID; //habilita semaphore
	semaphores[n_sem].counter = value; //asigna comptador
	INIT_LIST_HEAD(&semaphores[n_sem].blocked_list); //inicialitza llista
	return 0; //returns OK
}

int sys_sem_wait(int n_sem) {
	if (n_sem < 0 || n_sem > NR_SEM-1) return -EINVAL; //n_sem no valido
	if (semaphores[n_sem].owner == -1) return -EINVAL; //process alliberat
	if (--semaphores[n_sem].counter < 0) {
		list_add_tail(&current()->list, &semaphores[n_sem].blocked_list);
		sched_next_rr();
		if (semaphores[n_sem].owner == -1) return -1; //process alliberat
	}
	return 0; //return OK
}

int sys_sem_signal(int n_sem) {
	if (n_sem < 0 || n_sem > NR_SEM-1) return -EINVAL; //n_sem no valido
	if (semaphores[n_sem].owner == -1) return -EINVAL; //process alliberat
	if (++semaphores[n_sem].counter <= 0) {
		struct list_head *l = list_first(&semaphores[n_sem].blocked_list); //agafem següent
		list_del(l);
		list_add_tail(l, &readyqueue);
	}
	return 0; //return OK
}

int sys_sem_destroy(int n_sem) {
	if (n_sem < 0 || n_sem > NR_SEM-1) return -EINVAL; //n_sem no valido
	if (semaphores[n_sem].owner == -1) return -EINVAL; //si ya está usado
	
	if (semaphores[n_sem].owner == current()->PID) {
		while (++semaphores[n_sem].counter <= 0) {
			struct list_head *l = list_first(&semaphores[n_sem].blocked_list);
			list_del(l);
			list_add_tail(l, &readyqueue);
		}	
		semaphores[n_sem].owner = -1; //el deshabilitem
		return 0; //return OK
	}
	else return -EPERM;
}

void destroy_proc_semaphores() {
	int i;
	for (i = 0; i < NR_SEM; i++)
		if (current()->PID == semaphores[i].owner)
			sys_sem_destroy(i);
}

int sys_read(int fd, char *buff, int count) {
	int i = check_fd(fd, LECTURA);
	if (i != 0) return i; 
	if (buff == NULL) return -EFAULT;
	if (count <= 0) return -EINVAL;
	if (!access_ok(VERIFY_WRITE, buff, count)) return -EFAULT;
	current()->remaining = count;
	return sys_read_keyboard(buff, count);
}

void * sys_sbrk(int increment) {
	void * brk = current()->brk;
	page_table_entry *pte = get_PT(current());
	int i;
	if (increment >= 0) { //add dynamic memory
		int frames[HEAP_MAX_PAGE];
		unsigned int new_frame;
		unsigned int allocate;
		unsigned int brk_page;
		
		if (increment > 0 && increment < PAGE_SIZE && ((unsigned int)brk>>12) ==
			HEAP_TOP_PAGE) {
			allocate = (((unsigned int)brk + increment)>>12) - 
				((unsigned int) brk>>12) + 1;
			brk_page = (unsigned int) brk >> 12;
		}
		else {
			allocate = (((unsigned int)brk+increment)>>12)-((unsigned int)brk>>12);
			brk_page = ((unsigned int) brk>>12)+1;
		}
		for (i = 0; i < allocate; i++) {
			new_frame = alloc_frame();
			if (new_frame == -1) break;
			frames[i] = new_frame;
		}
		if (i < allocate) {
			while (i >= 0) {
				free_frame(frames[i]);
				i--;
			}
			return -ENOMEM;
		}
		i = 0;
		while (i < allocate) {
			set_ss_pag(pte, brk_page++,frames[i++]); 
		}
		current()->brk +=increment;
	}
	else if (increment < 0) {
		unsigned int last_page = (unsigned int)brk>>12;
		unsigned int desallocate = last_page - (((unsigned int)brk + increment)>>12);
		for (i = 0; i <= desallocate && last_page >= HEAP_TOP_PAGE; i++) {
			free_frame(get_frame(pte, last_page));
			del_ss_pag(pte, last_page--);
		}
		set_cr3(get_DIR(current()));
		if ((((unsigned int)brk + increment)>>12) < HEAP_TOP_PAGE) 
			brk = HEAP_TOP_PAGE << 12;
		else brk += increment;
		current()->brk = brk;
	}
	return brk;
}




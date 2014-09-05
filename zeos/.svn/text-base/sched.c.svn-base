/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));



struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry(l, struct task_struct, list);
}
 
int allocDIR[NR_TASKS];
struct list_head blocked;
struct list_head readyqueue;
struct list_head freequeue;
struct  task_struct *idle_task;
struct task_struct *task1;
int nPID;
struct sem semaphores[NR_SEM];
struct list_head keyboardqueue;
char cbuffer[MAX_BUFFER];
int cread;
int cwrite;
int compt;

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) //process
{
	int i;
	for(i = 0; i < NR_TASKS; i++) {
		if (allocDIR[i] == 0) {
			t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[i];
			allocDIR[i]++;
			return 0;
		}
	}
	return -1;
}

void update_DIR() { //thread
#if 0	
	int i;
	for (i = 0; i < NR_TASKS; i++) 
		if ((page_table_entry*)&dir_pages[i] == get_DIR(current()))
			allocDIR[i] += 1;
}
#else
	int pos = (int)((current()->dir_pages_baseAddr-&dir_pages[0][0])/
		(sizeof(page_table_entry) *TOTAL_PAGES));
	allocDIR[pos]++;
#endif
}

void desallocate_DIR() //process && thread
{
	int i;
	int pos = (int)((current()->dir_pages_baseAddr-&dir_pages[0][0])/
		(sizeof(page_table_entry) *TOTAL_PAGES));
	allocDIR[pos]--;
	if (allocDIR[pos] == 0) {
		unsigned int NUM_PAG = ((unsigned int)current()->brk >> 12) -
			(NUM_PAG_KERNEL + NUM_PAG_CODE);
        if((unsigned int)current()->brk > (NUM_PAG_KERNEL + NUM_PAG_CODE + 
        	NUM_PAG_DATA)<<12)	NUM_PAG++;
		for (i = 0; i < NUM_PAG; ++i) //free mem physical
		free_frame(get_frame(get_PT(current()), PAG_LOG_INIT_DATA_P0+i));
	}
}
void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");
		printk("CPU_IDLE\n");
	while(1) {}
}

void init_idle (void)
{
	union task_union * idle = (union task_union *)list_head_to_task_struct(list_first(&freequeue));
	list_del(&idle->task.list); //delete idle de freequeue
	
	idle->task.PID = 0;
	idle->task.quantum = QUANTUM;
	idle->task.brk = NULL;
	allocate_DIR(&idle->task); //falta mirar el valor de retorn
	idle->stack[KERNEL_STACK_SIZE-1] = (unsigned int) cpu_idle; //@ret, while(1)
	idle->stack[KERNEL_STACK_SIZE-2] = (unsigned int) 0; //ebp = 0, while(1)
	idle->task.kernel_esp = (unsigned int) &(idle->stack[KERNEL_STACK_SIZE-2]);
	
	idle_task = &idle->task;
}

void init_task1(void)
{
	task1 = list_head_to_task_struct(list_first(&freequeue));
	list_del(&task1->list); //delete task1 de freequeue

	task1->PID = 1;	nPID = 2;
	task1->quantum = current_quantum;
	task1->brk = (unsigned int )(NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA)<<12;
	/*Initialization of statistics*/
	update_stats(task1);
	
	allocate_DIR(task1); //falta mirar el valor de retorn
	set_user_pages(task1);

	union task_union *task1_union = (union task_union *) task1; //per accedir a pila
	tss.esp0 = (unsigned int)&(task1_union->stack[KERNEL_STACK_SIZE]);
	set_cr3(get_DIR(task1));
}


void init_sched(){
	INIT_LIST_HEAD(&freequeue);
	INIT_LIST_HEAD(&readyqueue);
	INIT_LIST_HEAD(&keyboardqueue);
	cwrite = cread = compt = 0;
	int i = 0;
		current_quantum = QUANTUM;
	/*posem tots els possibles task_structs a la llista de free*/
	while (i < NR_TASKS) {
		list_add(&task[i].task.list,&freequeue);
		allocDIR[i] = 0;
    	i++; 
	}
	i = 0;
	while (i< NR_SEM) {
		semaphores[i].owner = -1;
		i++;
	}
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
		:"=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

void inner_task_switch(union task_union *new) {
	tss.esp0 = (DWord) &(new->stack[KERNEL_STACK_SIZE]); //new kernel_stack
	set_cr3(get_DIR(&new->task)); //flush TLB
	__asm__ __volatile__(
		"movl %%ebp, %0;" //para old
		"movl %1, %%esp;"	//para new
		"popl %%ebp;"
		"ret;"
		:"=g" (current()->kernel_esp)
		:"g"(new->task.kernel_esp)
	);

}

void task_switch(union task_union *new) 
{
	__asm__ __volatile__(
		"pushl %esi;"
		"pushl %edi;"
		"pushl %ebx;"
	);
	inner_task_switch(new);
	__asm__ __volatile__(
		"popl %ebx;"
		"popl %edi;"
		"popl %esi;"
	);
}

void update_sched_data_rr() 
{
/*updates the number of ticks that the process has executed since it got assigned the cpu*/
	struct task_struct *current_ts = current();
	current_quantum--;
	current_ts->statistics.remaining_ticks = current_quantum;	
}

int needs_sched_rr()
{
	if (current_quantum <= 0) return 1;
	return 0;
}

void update_current_state_rr(struct list_head *dest)
{
	struct task_struct *current_ts = current();
	list_add_tail(&current_ts->list, dest);
}

void sched_next_rr()
{
	struct task_struct *next_ts;
	if (!list_empty(&readyqueue)) {
		/*obtenim el següent procés*/
		next_ts = list_head_to_task_struct(list_first(&readyqueue));
		/*assignem tickinicial = quantum*/
		current_quantum = next_ts->quantum;
		next_ts->statistics.remaining_ticks = current_quantum;
		/*eliminem el procés de la readyqueue, que passarà a la run*/
		list_del(list_first(&readyqueue));
		update_ready_exit(next_ts); //changes to system with new process
	}
	else next_ts = idle_task; //en cas que no tinguem processos pendents
	task_switch( (union task_union*) next_ts); //TASK SWITCH
}

int get_quantum(struct task_struct *t)
{
	return t->quantum;
}

void set_quantum(struct task_struct *t, int new_quantum)
{
	t->quantum = new_quantum;
}

void update_user_exit()
{
	/*changes form user to system*/
	struct task_struct *current_ts = current();
	current_ts->statistics.user_ticks += (get_ticks()) - 
							(current_ts->statistics.elapsed_total_ticks);
	current_ts->statistics.elapsed_total_ticks = get_ticks();
}

void update_user_entry() 
{
	/*changes from system to user*/
	struct task_struct *current_ts = current();
	current_ts->statistics.system_ticks += (get_ticks()) -
							(current_ts->statistics.elapsed_total_ticks);
	current_ts->statistics.elapsed_total_ticks = get_ticks();
}

void update_ready_exit(struct task_struct *ts)
{
	/*changes from ready to system*/
	ts->statistics.ready_ticks +=(get_ticks()-ts->statistics.elapsed_total_ticks);
	ts->statistics.elapsed_total_ticks = get_ticks();
	++(ts->statistics.total_trans);
}

void update_ready_entry()
{
	/*changes from system to ready*/
	struct task_struct *current_ts = current();
	current_ts->statistics.system_ticks += (get_ticks()) -
							(current_ts->statistics.elapsed_total_ticks);
	current_ts->statistics.elapsed_total_ticks = get_ticks();
}

void update_stats(struct task_struct *t) {
	t->statistics.remaining_ticks = current_quantum;
	t->statistics.user_ticks = 0;
	t->statistics.system_ticks = 0;
	t->statistics.user_ticks = 0;
	t->statistics.blocked_ticks = 0;
	t->statistics.ready_ticks = 0;
	t->statistics.elapsed_total_ticks = get_ticks();
	t->statistics.total_trans = 0;
}

void block_process(int i) { //i == 0 -> add_tail, i == 1 -> head
	struct task_struct *t = current();
	if (i == 0) list_add_tail(&t->list, &keyboardqueue);
	else list_add(&t->list, &keyboardqueue);
	t->statistics.system_ticks += (get_ticks() - t->statistics.elapsed_total_ticks);
	t->statistics.elapsed_total_ticks = get_ticks;
	sched_next_rr();
}

void unblock_process() {
	if (!list_empty(&keyboardqueue)) {
		struct task_struct *t = list_head_to_task_struct(list_first(&keyboardqueue));
		list_del(&t->list);
		list_add(&t->list, &readyqueue);
		t->statistics.system_ticks += get_ticks() - t->statistics.elapsed_total_ticks;
		t->statistics.elapsed_total_ticks = get_ticks();
		if(needs_sched_rr()) {
			update_current_state_rr(&readyqueue);
			sched_next_rr();
		}
	}
}
void add_to_cbuffer(char c) {
	if (compt == MAX_BUFFER) return; //no se puede escribir más
	else if (cwrite == MAX_BUFFER) cwrite = 0; //vuelve al principio
	cbuffer[cwrite] = c;
	cwrite++;
	compt++;
}
char get_from_cbuffer() {
	if (cread == cwrite && compt == 0) return 'w'; //quiere leer más de lo que está escrito
	if (cread == MAX_BUFFER && cread != cwrite) cread = 0; //vuelve al principio
	char c  = cbuffer[cread];
	cread++;
	compt--;
	return c;
}

int full_buffer() {
	if (compt == MAX_BUFFER) return 1;
	return 0;
}


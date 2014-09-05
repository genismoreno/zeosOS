/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>
#include <stats.h>
#include <utils.h>

#define NR_TASKS      10
#define KERNEL_STACK_SIZE	1024
#define QUANTUM 10
#define STATS_SIZE 28
#define NR_SEM 20
#define MAX_BUFFER 120

//enum state_t { ST_RUN, ST_READY, ST_BLOCKED };

unsigned int current_quantum;

struct task_struct {
  int PID;			/* Process ID */
  page_table_entry * dir_pages_baseAddr; /*directory*/
	struct list_head list; /*free, ready or run list*/
  //enum state_t estat; /*RUN, BLOCKED or READY*/
	unsigned int quantum;	/*maximum process ticks*/
	unsigned int kernel_esp; /*ebp*/
	struct stats statistics; /*statistics for scheduling policy*/
	int remaining; //keyboard read remaining chars
	void * brk; //break pointer
};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};

extern union task_union task[NR_TASKS]; /* Vector de tasques */
extern int allocDIR[NR_TASKS];
extern struct list_head blocked;
extern struct list_head readyqueue;
extern struct list_head freequeue;
extern struct  task_struct *idle_task;
extern struct task_struct *task1;
extern int nPID;
extern struct sem semaphores[NR_SEM];
extern struct list_head keyboardqueue;
extern char cbuffer[MAX_BUFFER];
extern int cread;
extern int cwrite;
extern int compt;


#define KERNEL_ESP(t)       	(DWord) &(t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP       	KERNEL_ESP(&task[1])

/* Headers for process creation and initialization */
void init_task1(void);
void init_idle(void);
void init_sched(void);
void task_switch(union task_union*t);
void inner_task_switch(union task_union *new);
int ret_from_fork();
/* Headers for auxiliar functions */
void update_stats(struct task_struct *t);
struct task_struct * current();
struct task_struct *list_head_to_task_struct(struct list_head *l);
int allocate_DIR(struct task_struct *t);
void update_DIR();
void desallocate_DIR();
page_table_entry * get_PT (struct task_struct *t) ;
page_table_entry * get_DIR (struct task_struct *t) ;
void destroy_proc_semaphores();
/* Headers for the scheduling policy */
void sched_next_rr();
void update_current_state_rr(struct list_head *dest);
int needs_sched_rr();
void update_sched_data_rr();
int get_quantum(struct task_struct *t);
void set_quantum(struct task_struct *t, int new_quantum);

/* Headers for IO functions*/
void block_process(int i);
void unblock_process();
void add_to_cbuffer(char c);
char get_from_cbuffer();
int get_distance();
int full_buffer();

/* Header for statistics */
void update_user_exit();
void update_user_entry();
void update_ready_exit(struct task_struct *ts);
void update_ready_entry();

#endif  /* __SCHED_H__ */

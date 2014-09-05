#include <io.h>
#include <utils.h>
#include <list.h>
#include <errno.h>
#include <sched.h>

int sys_write_console(char *buffer,int size)
{
  int i;
  for (i=0; i<size; i++)
    printc(buffer[i]);
  return size;
}

int sys_read_keyboard(char *buff, int size) {
	struct task_struct *t = current();
	if (!list_empty(&keyboardqueue)) {
		block_process(0);
	}
	while (t->remaining > 0) {
		if (compt < t->remaining && compt != 0) {
			while (compt > 0) {
				char c = get_from_cbuffer();
				if (copy_to_user(&c, buff, 1) != 0) return -EFAULT;
				buff++;	t->remaining--;
			}
			block_process(1);		
		}
		else if (compt >= t->remaining) {
			while (t->remaining > 0) {
				char c = get_from_cbuffer();
				if (copy_to_user(&c, buff, 1) != 0) return -EFAULT;
				buff++; t->remaining--;
			}
		}
		else {
			block_process(1);
		}
	}
	return size;
}


                                                                             



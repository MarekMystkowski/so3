#include "inc.h"
#define MAX_PID 30000
#define MAX_KEY 1000

/* It allows you to use: semctl, semget. */
int _semget(key_t key, int nsems, int flag)
{
	message m;
	m.SEMGET_KEY = key;
	m.SEMGET_NR = nsems;
	m.SEMGET_FLAG = flag;
	int x =  do_semget(&m);
	if (x == OK)
	    	return m.SEMGET_RETID;
	return -1;
}

int _semctl(int id, int num, int cmd, long opt )
{
	message m;
	m.SEMCTL_ID = id;
	m.SEMCTL_NUM = num;
	m.SEMCTL_CMD = cmd;
	m.SEMCTL_OPT = opt;
	int x =  do_semctl(&m);
	if (x == OK)
		return m.SHMCTL_RET;
	return -1;
}

#define semget(x, y, z) _semget(x, y, z)
#define semctl(a, b, c, d) _semctl(a, b, c,(long) d)

struct proc {
	key_t key_sem;			
	int has;			/* whether the process has semaphores */
	
};

struct procsem {
	size_t how_many_uses;		/* how many processes use it */
	size_t length;                  /* semaphore array length */
	int id;
};

static struct proc tab_proc[MAX_PID+1];
static struct procsem tab_sem[MAX_KEY+1];

static key_t unused_key = 1;

static key_t create_sem(size_t lenght)		
{		
	int ok, i, x;
	key_t key;
	for (ok = 0, i = 0; !ok && i < MAX_KEY; i++) {
	    	key = (unused_key + i - 1) % MAX_KEY + 1;
		if (tab_sem[key].how_many_uses == 0 &&
			(x = semget(key, lenght, 0666 | IPC_CREAT | IPC_EXCL)) != -1)
			ok = 1;
	}
	if (ok) {
		unused_key = key % MAX_KEY + 1;
		tab_sem[key].length = lenght;
		tab_sem[key].id = x;
		return key;
	}
	return -1;
}

/* releases semaphores used by the process */
static int release_semaphores(pid_t pid)
{
	if (pid < 1) return OK;
	if (!tab_proc[pid].has) return OK;
	tab_proc[pid].has = 0;

	key_t key_sem = tab_proc[pid].key_sem;
	tab_sem[key_sem].how_many_uses--;
	if (tab_sem[key_sem].how_many_uses == 0) {
		// removing segment semaphore
	    	if (semctl(tab_sem[key_sem].id, 0, IPC_RMID, 0) == -1){
		    	return -1;
		}
		tab_sem[key_sem].id = 0;
		tab_sem[key_sem].length = 0;
	}
	return OK;
}

/*===========================================================================*
 *                              do_procsem_init                              *
 *===========================================================================*/
int do_procsem_init(message *m)
{
	key_t key;
	pid_t pid = m->PROCSEM_INIT_PID;
	size_t length = m->PROCSEM_INIT_SIZE;

	if (release_semaphores(pid) != OK) 
		printf("Error in do_procsem_init()\n");

	key = create_sem(length);
	if(key == -1)
	    	return -1 /*ESIZE*/;

	tab_sem[key].how_many_uses = 1;
	tab_sem[key].length = length;
	tab_proc[pid].key_sem = key;
	tab_proc[pid].has = 1;
	
	printf("IPC proc_sem: init key = %d, len = %d, pid = %d\n", key, length, pid);

	return OK;
}

/*===========================================================================*
 *                              do_procsem_get                               *
 *===========================================================================*/
int do_procsem_get(message *m)
{
	key_t key;
	pid_t pid = m->PROCSEM_GET_PID ;
	size_t index = m->PROCSEM_GET_INDEX;
	
	printf("IPC proc_sem: get index = %d, pid = %d\n", index, pid);

	if (!tab_proc[pid].has)
		return EINVAL;
	key =  tab_proc[pid].key_sem;
	if (index > tab_sem[key].length)  
		return EINVAL;

	m->PROCSEM_GET_KEY = key;

	printf("IPC proc_sem: get: return key = %d\n", key);

	return OK;
}

/*===========================================================================*
 *                              do_procsem_inherit                           *
 *===========================================================================*/
int do_procsem_inherit(message *m)
{
	pid_t pid_parent = m->PROCSEM_INHERIT_PID_PARENT ;
	pid_t pid_son = m->PROCSEM_INHERIT_PID_SON ;
	
	printf("IPC proc_inhe: parent = %d, son = %d\n", pid_parent, pid_son);
	
	tab_proc[pid_son].has = 0;
	tab_proc[pid_son].key_sem = 0;

	if (pid_parent < 1 || !tab_proc[pid_parent].has)
		return OK;
	tab_proc[pid_son].has = tab_proc[pid_parent].has;
	key_t key = tab_proc[pid_parent].key_sem;
	tab_proc[pid_son].key_sem = key;
	tab_sem[key].how_many_uses++;

	return OK;
}

/*===========================================================================*
 *                              do_procsem_exit                              *
 *===========================================================================*/
int do_procsem_exit(message *m)
{
    pid_t pid = m->PROCSEM_EXIT_PID;
    
    printf("IPC proc_exit: pid = %d\n", pid);
    
    if (release_semaphores(pid) != OK)
		printf("Error in do_procsem_exit()\n");

    return OK;
}

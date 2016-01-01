#define _SYSTEM 1
#define _MINIX 1

#include <sys/cdefs.h>
#include <lib.h>

#include <minix/rs.h>

#include <lib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <proc_sem.h>

static int send_message_to_IPS(int syscallnr, message *m)
{
	endpoint_t ipc_pt;
	if(minix_rs_lookup("ipc", ipc_pt) != OK)
	{
		errno = ENOSYS;
		return -1;
	}
	return (_syscall(ipc_pt, syscallnr, &m));
}

int proc_sem_init(size_t n)
{
	message m;
	m.PROCSEM_INIT_PID = getpid();
	m.PROCSEM_INIT_SIZE = n;
	if (send_message_to_IPS(IPC_PROCSEM_INIT, &m) != OK)
		return ESIZE;
	return 0;
}

static void send_and_opsem(size_t sem_nr, int op)
{
	message m;
	m.PROCSEM_GET_PID = getpid();
	m.PROCSEM_GET_INDEX = sem_nr;
	if (send_message_to_IPS(IPC_PROCSEM_GET, &m) != OK)
		return ;
	
	int id = semget(m.PROCSEM_GET_KEY, sem_nr + 1, 0);
	if (id == -1)
		return;
	
	struct sembuf buf[1];
	buf[0].sem_op = op;
	buf[0].sem_num = sem_nr;
	buf[0].sem_flg = 0;
	semop(id, buf, 1);
	
}

void proc_sem_post(size_t sem_nr)
{
	send_and_opsem(sem_nr, 1);
}

void proc_sem_wait(size_t sem_nr)
{
	send_and_opsem(sem_nr, -1);
}

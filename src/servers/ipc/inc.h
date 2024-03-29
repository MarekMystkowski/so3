#define _POSIX_SOURCE      1	/* tell headers to include POSIX stuff */
#define _MINIX             1	/* tell headers to include MINIX stuff */
#define _SYSTEM            1    /* get OK and negative error codes */

#include <minix/callnr.h>
#include <minix/com.h>
#include <minix/config.h>
#include <minix/ipc.h>
#include <minix/endpoint.h>
#include <minix/sysutil.h>
#include <minix/const.h>
#include <minix/type.h>
#include <minix/syslib.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <machine/vm.h>
#include <machine/vmparam.h>
#include <sys/vm.h>

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

int do_shmget(message *);
int do_shmat(message *);
int do_shmdt(message *);
int do_shmctl(message *);
int check_perm(struct ipc_perm *, endpoint_t, int);
void update_refcount_and_destroy(void);
int do_semget(message *);
int do_semctl(message *);
int do_semop(message *);
int do_procsem_init(message *);
int do_procsem_get(message *);
int do_procsem_inherit(message *);
int do_procsem_exit(message *);
int is_sem_nil(void);
int is_shm_nil(void);
void sem_process_vm_notify(void);

/* used by the proc_sem.c to manage semaphores with sem.c */
int do_remove_semaphore(int id);


EXTERN int identifier;
EXTERN endpoint_t who_e;
EXTERN int call_type;
EXTERN endpoint_t SELF_E;


#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#define ESIZE -10
int proc_sem_init(size_t n);
void proc_sem_post(size_t sem_nr);
void proc_sem_wait(size_t sem_nr);

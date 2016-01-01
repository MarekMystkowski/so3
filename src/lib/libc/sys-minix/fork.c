#include <sys/cdefs.h>
#include "namespace.h"
#include <lib.h>

#include <unistd.h>
#include <minix/rs.h>

#ifdef __weak_alias
__weak_alias(fork, _fork)
#endif

pid_t fork()
{
  message m;
  pid_t pid_parent, pid_son, result;
  pid_parent = getpid();
  
  result = _syscall(PM_PROC_NR, FORK, &m);
  if (result)
    return result;
   
   /* son */
  pid_son = getpid();
  
  /* sending message to server IPC */
  m.PROCSEM_INHERIT_PID_SON = pid_son;
  m.PROCSEM_INHERIT_PID_PARENT = pid_parent;
  endpoint_t ipc_ep;
  minix_rs_lookup("ipc", &ipc_ep);
  _syscall(ipc_ep, IPC_PROCSEM_INHERIT, &m);
  
  return result;
}

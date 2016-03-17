#include <sys/cdefs.h>
#include "namespace.h"
#include <lib.h>
#include <string.h>

#include <unistd.h>
#include "mq.h"

int mq_open (char *, int );
int mq_close (int);
int mq_send (char *, int, int, int *,int);
char *mq_receive (int);
int mq_setattr (int, int, int);
int mq_getattr (int);
int mq_reqnotify(int, int);

int mq_open(char *mqname, int max_msg)
{
    message m;
    m.m1_i1 = strlen(mqname) + 1;
    m.m1_i2 = max_msg;
    m.m1_p1 = (char *)mqname;

    return(_syscall(VFS_PROC_NR, VFS_MQ_OPEN, &m));
}

int mq_close(int mqid)
{
    message m;
    m.m1_i1=mqid;

    return(_syscall(VFS_PROC_NR, VFS_MQ_CLOSE, &m));
}

int mq_send(char *msg,int mqid, int sender_id , int *recv_list, int prio)
{
    message m;
    m.m1_i1 = mqid;
    m.m1_p1 = (char *)msg;
    m.m1_i2 = sender_id;
    m.m1_i3 = prio;

    /* 
     * Send recvr list also 
     */

    return(_syscall(VFS_PROC_NR, VFS_MQ_SEND, &m)); 
}

char * mq_receive(int mqid)
{
    message m;
    int r;
    m.m1_i1 = mqid;

    r = _syscall(VFS_PROC_NR, VFS_MQ_RECEIVE, &m);
    if(r != 0)
	return NULL;

    return NULL ; 
    //return(char *)m.VMM_RETADDR;
    /*Or m.VMRE_RETA; need to investigate..*/ 

}

int mq_setattr(int mqid, int maxmsgno, int blocking)
{
    message m;
    m.m1_i1 = mqid;
    m.m1_i2 = maxmsgno;
    m.m1_i3 = blocking;

    return(_syscall(VFS_PROC_NR, VFS_MQ_SETATTR, &m));
}

int mq_getattr(int mqid)
{
    message m;
    m.m1_i1 = mqid;

    return(_syscall(VFS_PROC_NR, VFS_MQ_GETATTR, &m));
}

int mq_reqnotify(int pid, int mqid)
{
    message m;
    m.m1_i1 = pid;
    m.m1_i2 = mqid;

    return(_syscall(VFS_PROC_NR, VFS_REQ_MQ_NOTIFY, &m));
}

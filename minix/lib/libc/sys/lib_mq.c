#include <sys/cdefs.h>
#include "namespace.h"
#include <lib.h>
#include <string.h>
#include<stdio.h>
#include <unistd.h>
#include "lib_mq.h"

int mq_open (char *, int );
int mq_close (int);
int mq_send (char *, int, int, int ,int);
char *mq_receive (int, int);
int mq_setattr (int, int, int);
int mq_getattr (int );
int mq_reqnotify(int, int);

char ret_msg[32];
/*
 * Returns mq_id 
 * If its -1 then mq_open failed 
 */
int mq_open(char *mqname, int max_msg)
{
    message m;
    strcpy(m.m_mq_open.name, mqname);
    m.m_mq_open.max_msg = max_msg;
    return(_syscall(VFS_PROC_NR, VFS_MQ_OPEN, &m));
}

/* 
 * Returns -1 on failure 
 * other vaules for success 
 */
int mq_close(int mqid)
{
    message m;
    m.m_mq_close.mqid = mqid;

    return(_syscall(VFS_PROC_NR, VFS_MQ_CLOSE, &m));
}
/*
 * Returns -1 for failure 
 * 0 for success 
 */
int mq_send(char *msg, int mqid, 
	    int sender_id , int recv, int prio)
{
    message m;
    m.m_mq_send.mqid = mqid;
    strcpy(m.m_mq_send.message, msg); 
    m.m_mq_send.sender_id = sender_id;
    m.m_mq_send.prio = prio;
    m.m_mq_send.recv = recv; 

    /* 
     * Send recvr list also 
     */

    return(_syscall(VFS_PROC_NR, VFS_MQ_SEND, &m)); 
}

char * mq_receive(int mqid , int recv_id)
{
    message m;
    int r;
    m.m_mq_receive.mqid = mqid; 
    m.m_mq_receive.recv_id = recv_id;

    r = _syscall(VFS_PROC_NR, VFS_MQ_RECEIVE, &m);
    if (r < 0) {
	return NULL;
    }

    strcpy(ret_msg, m.m_mq_receive.message);
    return ret_msg;
}

int mq_setattr(int mqid, int maxmsgno, int blocking)
{
    message m;
    m.m_mq_setattr.mqid     = mqid;
    m.m_mq_setattr.max_msg  = maxmsgno;
    m.m_mq_setattr.blocking = blocking;

    return(_syscall(VFS_PROC_NR, VFS_MQ_SETATTR, &m));
}

int mq_getattr(int mqid)
{
    int r;
    message m;
    m.m_mq_getattr.mqid = mqid;
    r = _syscall(VFS_PROC_NR, VFS_MQ_GETATTR, &m); 
    printf ("Maximum message for this queue = %d blocking = %s\n",
	    m.m_mq_getattr.max_msg , m.m_mq_getattr.blocking ? "yes":"no");
    return r;
}

int mq_reqnotify(int pid, int mqid)
{
    message m;
    m.m1_i1 = pid;
    m.m1_i2 = mqid;

    return(_syscall(VFS_PROC_NR, VFS_REQ_MQ_NOTIFY, &m));
}

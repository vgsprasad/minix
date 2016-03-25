 /*
 * The entry points into this file are
 * mq_send
 * mq_receive
 * mq_open
 * mq_close
 */

#include "fs.h"
#include <minix/callnr.h>
#include <minix/com.h>
#include <minix/u64.h>
#include <minix/vfsif.h>
#include <assert.h>
#include <sys/dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include "file.h"
#include "scratchpad.h"
#include "vnode.h"
#include "vmnt.h"
#include <string.h> 

/* Max number of receivers possible */ 
#define MAX_RECVR 100
/* Mark the message obsolete */
#define INVALID_RECVR -1  
/*Maximum message queues allowed */ 
#define MAX_MQ 100

/* Counts the total number of message queues */ 
int global_mq_count = 0; 

/* Message queue structure */ 
struct message_queue {
    char name[32];
    int mq_id;
    int max_msg;
    int blocking;
    struct message *head;
};

struct message_queue *mq[MAX_MQ];

/* Message structure */ 
struct message {
    char msg[32];
    int prio;
    int seq_num;
    int sender;
    int recv_id;
    struct message *next;
};

/* prototypes */
int 
add_message_to_queue(int mq_id, int sender, int recv_id,
		     int prio);
int do_mq_send(void);
int do_mq_receive(void);
int do_mq_open(void);
int do_mq_close(void);
int do_mq_setattr(void);
int do_mq_getattr(void);
int do_req_mqnotify(void);
struct message_queue * get_message_queue(int mq_id );

/*===========================================================================*
 *				mq_send					     *
 *===========================================================================*/
int do_mq_send(void)
{
    int ret_val;
    /*Initialize based on the arguments passed */
    int mq_id, recv_id, prio, sender_id; 

    mq_id = m_in.m_mq_send.mqid;
    recv_id = m_in.m_mq_send.recv;
    prio = m_in.m_mq_send.prio;
    sender_id = m_in.m_mq_send.sender_id;
    /* 
     * We got mqid , sender id , recv id and priority info
     * Store it in message structure for future reference 
     */ 
    ret_val = add_message_to_queue(mq_id, sender_id, recv_id, prio);
    return ret_val;
}


/*===========================================================================*
 *				mq_receive				     *
 *===========================================================================*/
int do_mq_receive(void)
{
    int mq_id, prio_msg;
    struct message *temp ,*prio_temp;
    struct message_queue *temp_mq;
    int recv_id ;
    

    mq_id = m_in.m_mq_receive.mqid;
    recv_id = m_in.m_mq_receive.recv_id;
    temp_mq = get_message_queue(mq_id);

    if (temp_mq == NULL ) { 
	printf(" Unable to get message queue id \n");
	return -1;
    }
    temp = temp_mq->head;

    /*
     * Implementation of fetching priority message 
     * Negative priority is least . Not possible . 
     * All priorities assumed +ve 
     */ 
    prio_temp = temp; 
    prio_msg = -1; 

    while (temp)
    {
	/* 
	 * Find a message which is of high priority
	 */
	if ((temp->recv_id == recv_id) && (temp->prio > prio_msg)) {
	    prio_temp = temp;
	    prio_msg  = temp->prio; 
	}
	temp = temp ->next;
    }
    if (prio_temp) {
	/* 
	 * Send the highest priority message 
	 */
	prio_temp->recv_id= INVALID_RECVR;
	strcpy(job_m_out.m_mq_receive.message, prio_temp->msg);
	return 0;
    }
	
    /* 
     * If there are no messages return -1 
     * Means there are no messages intended for this reciever 
     */
    return -1;
}

/*===========================================================================*
 *				mq_open					     *
 *===========================================================================*/
int do_mq_open(void)
{
    /* 
     * Local message queue id used for statistical purpose 
     */
    static int mq_id = -1;
    /*
     * Initially all are non blocking 
     */
    int blocking = 0;
    int index =0;

    /* 
     * If there are already opened message queues return the same mqid 
     * without creating new one 
     */ 
    for (index =0; index < global_mq_count; index++) {
	if (!strcmp(mq[index]->name, m_in.m_mq_open.name)) {
	    return mq[index]->mq_id;
	}
    }

    mq_id++;

    /*
     * Open message queue
     */
    mq[mq_id] = (struct message_queue *) malloc(sizeof(struct message_queue));
    if (mq[mq_id] == NULL) {
	printf(" Unable to allocate memory \n");
	return -1;
    }

    /*
     * Store message queue attributes 
     */
    global_mq_count++;
    mq[mq_id]->mq_id = mq_id;
    mq[mq_id]->max_msg = m_in.m_mq_open.max_msg;
    strcpy(mq[mq_id]->name, m_in.m_mq_open.name); 
    mq[mq_id]->blocking = blocking;
    mq[mq_id]->head = NULL;
    /*
     * Return message queue ID 
     */
    return mq_id;
    
}

/*===========================================================================*
 *				mq_close				     *
 *===========================================================================*/
int do_mq_close(void)
{
    int mq_id = m_in.m_mq_close.mqid;
    struct message_queue *temp_mq;
    /* 
     * Find a message queue id from the list of opened message queues 
     */
    temp_mq = get_message_queue(mq_id);
    if (!temp_mq) {
	printf(" Mq id not found return error \n");
	return -1;
    }

    /*
     * Add a code to NULLify mq[index] otherwise it might core .
     */
    free(temp_mq) ;
    return 1;
}

/*===========================================================================*
 *				mq_getattr				     *
 *===========================================================================*/
int do_mq_getattr(void)
{
    int mq_id;
    struct message_queue *temp_mq;
    mq_id = m_in.m_mq_getattr.mqid; 
    temp_mq = get_message_queue(mq_id);
    if (!temp_mq) {
	return -1;
    }
    /* Get attributes of message queue */ 
    job_m_out.m_mq_getattr.max_msg = temp_mq->max_msg;
    job_m_out.m_mq_getattr.blocking = temp_mq->blocking;
    
    return 1;
}


/*===========================================================================*
 *				mq_setattr				     *
 *===========================================================================*/
int do_mq_setattr(void)
{
    int mq_id;

    struct message_queue *temp_mq;
    mq_id = m_in.m_mq_setattr.mqid; 
    temp_mq = get_message_queue(mq_id);
    if (!temp_mq) {
	return -1;
    }
    /* Set attributes of the message queue */
    temp_mq->max_msg = m_in.m_mq_setattr.max_msg;
    temp_mq->blocking = m_in.m_mq_setattr.blocking;
    return 1;
}


/*===========================================================================*
 *				req_mq_notify				     *
 *===========================================================================*/
int do_req_mq_notify(void)
{
    return 1;
}

struct message_queue *
get_message_queue(int mq_id )
{
    int index = 0;
    while ((index < global_mq_count))
    {
	if (mq[index]->mq_id == mq_id) {
	    return mq[index];
	}
	index++;
    }
    return NULL;
}

int
add_message_to_queue(int mq_id, int sender, int recv_id,
		     int prio)
{
    struct message *temp , *temp1;
    struct message_queue *temp_mq;
    static int seq_num; 

    /* 
     * Get the exact message queue id 
     */
    temp_mq = get_message_queue(mq_id);
    if (!temp_mq) {
	printf("Unable to add message to message queue . Wrong mq_id \n");
	return -1;
    }

    temp = (struct message *)malloc(sizeof(struct message ));
    if (!temp ) {
	printf("Unable to allocate memory for message \n");
	return -1;
    }
    /*
     * Copy the message content into message node 
     */
    strcpy(temp->msg, m_in.m_mq_send.message);
    temp->prio    = prio;
    temp->seq_num = seq_num++;
    temp->sender  = sender;
    temp->recv_id = recv_id;
    temp->next    = NULL;

    temp1 = temp_mq->head;
    /*
     * Intelligent code acts as queue . 
     * Whenever new message comes it adds it in the tail 
     */
    if (temp1 == NULL) {
	temp_mq->head = temp;
	temp_mq->head->next = NULL;
    } else {
	while (temp1->next != NULL) {
	    temp1 = temp1->next;
	}
	if (temp1 != NULL) {
	    temp1->next =temp;
	}
    }
    return 0;
}

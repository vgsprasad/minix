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

#define MAX_RECVR 100
#define INVALID_RECVR 0
#define MAX_MQ 100

int global_mq_count = 0; 

struct message_queue {
    char name[32];
    int mq_id;
    int max_msg;
    int blocking;
    struct message *head;
};

struct message_queue *mq[MAX_MQ];

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
void check_message_for_deletion(struct message_queue *temp_mq,
				struct message *temp);
void delete_message_from_queue(struct message_queue *temp_mq,
			       struct message *temp );
struct message_queue * get_message_queue(int mq_id );

/*===========================================================================*
 *				mq_send					     *
 *===========================================================================*/
int do_mq_send(void)
{
    int ret_val;
    printf("Called mq_send \n");
    /*Initialize based on the arguments passed */
    int mq_id, recv_id, prio, sender_id; 

    mq_id = m_in.m_mq_send.mqid;
    recv_id = m_in.m_mq_send.recv;
    prio = m_in.m_mq_send.prio;
    sender_id = m_in.m_mq_send.sender_id;

    printf(" Call add_message_to_queue \n");
    ret_val = add_message_to_queue(mq_id, sender_id, recv_id, prio);
    return ret_val;
}


/*===========================================================================*
 *				mq_receive				     *
 *===========================================================================*/
int do_mq_receive(void)
{
    int mq_id;
    struct message *temp ;
    struct message_queue *temp_mq;
    int recv_id ;
    

    mq_id = m_in.m_mq_receive.mqid;
    recv_id = m_in.m_mq_receive.recv_id;
    temp_mq = get_message_queue(mq_id);

    if (temp_mq == NULL ) { 
	printf(" Unable to get message queue id \n");
	return -1;
    }
    temp = temp_mq->head ;

    while (temp)
    {
	/*
	 * If receiver list is allowed loop here 
	 */
	if (temp->recv_id == recv_id ) {
	    //temp->recv_list[index] = INVALID_RECVR;
	    //check_message_for_deletion(temp_mq, temp);
	    printf("Found message %s  \n", temp->msg);
	    strcpy(job_m_out.m_mq_receive.message, temp->msg);
	    return 0;
	}
	temp = temp ->next;
    }
    return -1;
}

/*===========================================================================*
 *				mq_open					     *
 *===========================================================================*/
int do_mq_open(void)
{
    static int mq_id = -1;
    int blocking = 0;
    int index =0;

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

    printf(" created new message queue \n");
    global_mq_count++;
    mq[mq_id]->mq_id = mq_id;
    mq[mq_id]->max_msg = m_in.m_mq_open.max_msg;
    strcpy(mq[mq_id]->name, m_in.m_mq_open.name); 
    mq[mq_id]->blocking = blocking;
    mq[mq_id]->head = NULL;
    printf(" Return mq_id  %d \n", mq_id);
    return mq_id;
    
}

/*===========================================================================*
 *				mq_close				     *
 *===========================================================================*/
int do_mq_close(void)
{
    int mq_id = m_in.m_mq_close.mqid;
    struct message_queue *temp_mq;
    temp_mq = get_message_queue(mq_id);
    if (!temp_mq) {
	printf(" Mq id not found return error \n");
	return -1;
    }

    /*
     * Add a code to NULLify mq[index] otherwise it might core .
     */
    printf(" free message queue \n");
    free(temp_mq) ;
    return 1;
}

/*===========================================================================*
 *				mq_getattr				     *
 *===========================================================================*/
int do_mq_getattr(void)
{
    int mq_id = m_in.m1_i1;
    struct message_queue *temp_mq;
    temp_mq = get_message_queue(mq_id);
    if (!temp_mq) {
	return 0;
    }
    /* 
     * Return values 
    *max_msg = temp_mq->max_msg;
    *blocking = temp_mq->blocking;
    */
    return 1;
}


/*===========================================================================*
 *				mq_setattr				     *
 *===========================================================================*/
int do_mq_setattr(void)
{
    int mq_id = m_in.m1_i1;
    int max_msg = m_in.m1_i2;
    int blocking = m_in.m1_i3;

    struct message_queue *temp_mq;
    temp_mq = get_message_queue(mq_id);
    if (!temp_mq) {
	return 0;
    }
    temp_mq->max_msg = max_msg;
    temp_mq->blocking = blocking;
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
	    printf("Found a message queue \n");
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

    temp_mq = get_message_queue(mq_id);
    if (!temp_mq) {
	printf("Unable to add message to message queue . Wrong mq_id \n");
	return -1;
    }

    temp = (struct message *)malloc(sizeof(struct message *));
    if (!temp ) {
	printf("Unable to allocate memory for message \n");
	return -1;
    }
    strcpy(temp->msg, m_in.m_mq_send.message);
    temp->prio = prio;
    temp->seq_num = seq_num++;
    temp->sender = sender;
    temp->recv_id = recv_id;
    temp->next = NULL;

    printf(" Temporary message created with the values sent \n");
    temp1 = temp_mq->head;
    if (temp1 == NULL) {
	temp_mq->head = temp;
	printf(" First message %u\n", (unsigned int )temp_mq->head);
    } else {
	printf(" temp1 = %u \n", (unsigned int )temp1);
	while (temp1->next != NULL) {
	    printf("While loop \n");
	    temp1 = temp1->next;
	}
	if (temp1 != NULL) {
	    temp1->next =temp;
	}
    }
    printf(" Stored the message in message queue now return \n");
    return 0;
}

void delete_message_from_queue(struct message_queue *temp_mq,
			       struct message *temp )
{
    struct message *t ;
    t = temp_mq->head;

    if (!t)
	return;
    if (t == temp ) {
	//mq->head = temp->next;
	free(temp);
    }
    while(t->next != temp) {
	t = t->next ;
    }
    if (t->next == temp ) {
	t->next = temp->next ;
	free(temp);
    }
}
#if 0
void check_message_for_deletion (struct message_queue * temp_mq,
				struct message *temp )
{
    int index;
    for (index = 0 ; index < MAX_RECVR; index ++)
    {
	if (temp->recv_list[index] != INVALID_RECVR)
	{
	    /*
	     * This ensures that atleast one recvr is waiting 
	     */
	    return;
	}
    }
    delete_message_from_queue(temp_mq, temp);
}
#endif 


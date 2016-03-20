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
    char *name;
    int mq_id;
    int max_msg;
    int blocking;
    struct message *head;
};

struct message_queue *mq[MAX_MQ];

struct message {
    char *msg;
    int prio;
    int seq_num;
    int sender;
    int recv_id;
    struct message *next;
};

/* prototypes */
int 
add_message_to_queue(int mq_id, char *message, int sender, int recv_id,
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
    char *message = NULL;
    printf("Called mq_send \n");
    /*Initialize based on the arguments passed */
    int mq_id = m_in.m1_i1; 
    int sender_id = m_in.m1_i2;
    int recv_id = 1;
    int prio = m_in.m1_i3;
    printf(" Message sent from application = %s \n", m_in.m1_p1);
    strlcpy(message, m_in.m1_p1, 10 );

    printf(" Call add_message_to_queue \n");
    ret_val = add_message_to_queue(mq_id, message, sender_id, recv_id, prio);
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
    

    mq_id = m_in.m1_i1;
    recv_id = m_in.m1_i2;
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
	    m_in.m1_p1 = temp->msg;
	    break;
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
    //strcpy(mq[mq_id]->name, m_in.m1_p1);
    mq[mq_id]->max_msg = m_in.m1_i2;
    mq[mq_id]->blocking = blocking;

    printf(" Return mq_id \n");
    return mq_id;
    
}

/*===========================================================================*
 *				mq_close				     *
 *===========================================================================*/
int do_mq_close(void)
{
    int mq_id = m_in.m1_i1;
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
add_message_to_queue(int mq_id, char *message, int sender, int recv_id,
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
    printf(" Message stored in the node = %s", message);
    temp->msg  = message;
    temp->prio = prio;
    temp->seq_num = seq_num++;
    temp->sender = sender;
    temp->next = NULL;
    temp->recv_id = recv_id;

    printf(" Temporary message created with the values sent \n");
    temp1 = temp_mq->head;
    if (temp1 == NULL) {
	temp_mq->head = temp;
    } else {
	while (temp1->next) {
	    temp1 = temp1->next;
	}
	temp1->next =temp;
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


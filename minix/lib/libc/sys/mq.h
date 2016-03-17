int mq_open (char *, int );
int mq_close (int);
int mq_send (char *, int, int, int *,int);
char * mq_receive (int);
int mq_setattr (int, int, int);
int mq_getattr (int);
int mq_reqnotify(int, int);

#include "fs.h"
#include "inode.h"

int fs_tool()
{
    int i;
    struct inode *ip;

    ip = get_inode(fs_dev,m_in.m_inode.inode_num);

    printf("printing zone info...\n");
    for(i=0;i<V2_NR_TZONES;i++)
	printf("%d \t",ip->i_zone[i]);

    return(OK);
}

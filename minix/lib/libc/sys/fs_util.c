#include <sys/cdefs.h>
#include "namespace.h"
#include <lib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

struct inode * get_inode_str(ino_t );

struct inode * get_inode_str(ino_t inode_num)
{
    message m;
    m.m_inode.inode_num = inode_num;
    return(_syscall(VFS_PROC_NR, VFS_INODE, &m));
}

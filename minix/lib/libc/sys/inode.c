#include <sys/cdefs.h>
#include "namespace.h"
#include <lib.h>
#include <string.h>
#include<stdio.h>
#include <unistd.h>

void * dump_zone_info(uint64_t ,int );

void * dump_zone_info( uint64_t num, int fd)
{
    int ret ;
    message m;
    void *ptr;
    m.m_vfs_fs_dump_zone_info.num = num;
    m.m_vfs_fs_dump_zone_info.fd = fd;
    ret = _syscall(VFS_PROC_NR, VFS_DUMP_ZONE_INFO, &m);
    return ptr;
}


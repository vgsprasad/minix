int do_fs_inode(void)
{
    message m;

    /* Fill in request message */
    m.m_type = REQ_INODE;

    /* Send/rec request */
    return fs_sendrec(fs_e, &m);
}

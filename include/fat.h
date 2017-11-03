#ifndef __FAT_H
#define __FAT_H

#include "vfs.h"

#define PATHN_MAX 256
#define SFN_MAX     12
#define ARG_MAX    32

/* open */
#define O_RDONLY 0x00
#define O_WRONLY 0x01
#define O_RDWR   0x02
#define O_ACCMODE 0x03

#define O_CREAT     000001000
#define O_EXCL      000002000
#define O_NOCTTY    000004000
#define O_TRUNC     000010000
#define O_APPEND    000020000
#define O_NONBLOCK  000040000

/* seek */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

int fatfs_mount(char *source, char *tgt, uint32_t flags, void *arg);
int fatfs_open(const char *path, int flags);
int fatfs_creat(struct fnode *fno);
int fatfs_read(struct fnode *fno, void *buf, unsigned int len);
int fatfs_write(struct fnode *fno, const void *buf, unsigned int len);
int fatfs_seek(struct fnode *fno, int off, int whence);
int fatfs_truncate(struct fnode *fno, unsigned int len);
int fatfs_unlink(struct fnode *fno);
int fatfs_close(struct fnode *fno);

#endif

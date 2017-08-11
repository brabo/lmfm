#ifndef __FAT_H
#define __FAT_H

#include "vfs.h"

struct fatfs_disk {
    struct fnode *blockdev;
    struct fnode *mountpoint;
    struct fatfs *fs;
};

struct fatfs_priv {
    uint32_t            sclust;
    uint32_t            cclust;
    uint32_t            sect;
    uint32_t            off;
    uint32_t            dirsect;
    struct fatfs_disk   *fsd;
    uint32_t            *fat;
};

struct fatfs {
    int         fd;
    uint8_t     win[512];
    uint32_t    bsect;
    uint8_t     type;
    uint16_t    bps;
    uint8_t     spc;
    uint32_t    database;
    uint32_t    fatbase;
    uint32_t    dirbase;
    uint32_t    n_fatent;
    uint8_t     mounted;
};

struct fatfs_dir {
    uint8_t     *fn;
    uint32_t    sclust;
    uint32_t    cclust;
    uint32_t    sect;
    uint32_t    off;
    uint32_t    dirsect;
    uint32_t    attr;
    uint32_t    fsize;
};

/* Constants */

/* move to limits.h ? */
#define MAXPATHLEN 256
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

int print_array(uint8_t *buf, int len);
int fatfs_mount(char *source, char *tgt, uint32_t flags, void *arg);
int fatfs_create(struct fnode *fno);
int fatfs_read(struct fnode *fno, void *buf, unsigned int len);
int fatfs_write(struct fnode *fno, const void *buf, unsigned int len);
int fatfs_seek(struct fnode *fno, int off, int whence);
int fatfs_close(struct fnode *fno);

#endif

#ifndef __FROSTED_H
#define __FROSTED_H

/*
 * Purely for what we need to provide to make fat.c ad verbatim identical to the one in frosted
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "fat.h"
#include "vfs.h"
#include "mockblock.h"

static int print_array(uint8_t *buf, int len)
{
    char *bugger = malloc(sizeof(uint8_t) * ((len * 2) + 3));
    char *ptr = bugger;
    uint8_t *ptr2 = buf;
    sprintf(ptr++, "[");

    for (int i = 0; i < len; i++) {
        sprintf(ptr, "%02X", *ptr2++);
        ptr += 2;
    }

    sprintf(ptr++, "]");
    printf("Read sector: %s\n", bugger);

    free(bugger);

    return 0;
}

#define kalloc(s) malloc(s)
#define kcalloc(n,s) calloc(s,n)
#define krealloc(p,s) realloc(p,s)
#define kfree(p) free(p)
#define task_filedesc_add(f) 3
#define register_module(m) 0

#define FAMILY_FILE 0xFFFF
#define MODNAME_SIZE 32
#define CONFIG_FAT32 1

struct module {
    uint16_t family;
    char name[MODNAME_SIZE];

    /* If this is a filesystem related module, it should probably define these */
    int (*mount)(char *source, char *target, uint32_t flags, void *arg);
    int (*umount)(char *target, uint32_t flags);
    int (*mount_info)(struct fnode *fno, char *buf, int size);

    struct module_operations {
        /* Common module operations */
        int (*read) (struct fnode *fno, void *buf, unsigned int len);
        int (*write)(struct fnode *fno, const void *buf, unsigned int len);
        int (*poll) (struct fnode *fno, uint16_t events, uint16_t *revents);
        int (*close)(struct fnode *fno);
        int (*ioctl)(struct fnode *fno, const uint32_t cmd, void *arg);

        /* Files only (NULL == socket) */
        int (*open)(const char *path, int flags);
        int (*seek)(struct fnode *fno, int offset, int whence);
        int (*creat)(struct fnode *fno);
        int (*unlink)(struct fnode *fno);
        int (*truncate)(struct fnode *fno, unsigned int size);
        void * (*exe)(struct fnode *fno, void *arg);

        /* Block device operations */
        int (*block_read)(struct fnode *fno, void *buf, uint32_t sector, int offset, int count);
        int (*block_write)(struct fnode *fno, const void *buf, uint32_t sector, int offset, int count);

    } ops;
    struct module *next;
};


#endif

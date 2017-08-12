#ifndef __VFS_H
#define __VFS_H

#define FL_RDONLY 0x01
#define FL_WRONLY 0x02
#define FL_RDWR   (FL_RDONLY | FL_WRONLY)
#define FL_DIR    0x04
#define FL_INUSE  0x08
#define FL_TTY    0x10
#define FL_BLK    0x20

#define FL_EXEC   0x40
#define FL_LINK   0x80

#ifndef FD_CLOEXEC
    #define FD_CLOEXEC  1
#endif

#ifndef F_DUPFD
    #define F_DUPFD 0
#endif

#ifndef F_GETFD
    #define F_GETFD 1
#endif

#ifndef F_SETFD
    #define F_SETFD 2
#endif

#ifndef F_GETFL
    #define F_GETFL 3
#endif

#ifndef F_SETFL
    #define F_SETFL 4
#endif

#define MAX_FILE 64

struct fnode {
    struct module *owner;
    char *fname;
    char *linkname;
    uint32_t flags;
    struct fnode *parent;
    struct fnode *children;
    void *priv;
    uint32_t size;
    uint32_t off;
    int32_t usage_count;
    struct fnode *next;
    int fd;
};

struct mountpoint
{
    struct fnode *target;
    struct mountpoint *next;
};

#include <sys/stat.h>

struct fnode *fno_search(const char *_path);
struct fnode *fno_create(struct module *owner, const char *name, struct fnode *parent);
struct fnode *fno_mkdir(struct module *owner, const char *name, struct fnode *parent);
int fno_fullpath(struct fnode *f, char *dst, int len);
void fno_unlink(struct fnode *fno);

struct fnode *vfs_opendir(void *arg1);
int vfs_readdir(void *arg1, void* arg2, void *arg3);
int vfs_closedir(void *arg1);
int vfs_stat(char *path, struct stat *st);
void vfs_init(void);
int vfs_mount(char *source, char *target, char *module, uint32_t flags, void *args);
struct fnode *vfs_open(void *arg1, uint32_t arg2);
int vfs_read(struct fnode *fno, void *buf, int len);
int vfs_seek(struct fnode *fno, int off, int whence);
int vfs_touch(char *path);

#endif

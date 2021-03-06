/*
 *      vfs.c: virtual filesystem for lmfm - ported from frosted
 *
 *      This is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as
 *      published by the Free Software Foundation.
 *
 *
 *      This is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with frosted.  If not, see <http://www.gnu.org/licenses/>.
 *
 *      Authors: brabo, Daniele Lacamera, Maxime Vincent
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include "fat.h"
#include "vfs.h"
#include "frosted.h"

#define P_EXEC     0000001   // exec

struct mountpoint *MTAB = NULL;

/* ROOT entity ("/")
 *.
 */
static struct fnode FNO_ROOT = {
};

static void basename_r(const char *path, char *res)
{
    char *p;
    strncpy(res, path, strlen(path) + 1);

    p = res + strlen(res) - 1;
    while (p >= res) {
        if (*p == '/') {
            *p = '\0';
            break;
        }
        p--;
    }

    if (strlen(res) == 0) {
        res[0] = '/';
        res[1] = '\0';
    }
}

static char *filename(char *path)
{
    int len = strlen(path);

    char *p = path + len - 1;
    while (p >= path) {
        if (*p == '/')
            return (p + 1);
        p--;
    }

    return path;
}

static struct fnode *_fno_search(const char *path, struct fnode *dir, int follow);
struct fnode *fno_search(const char *_path);
static struct fnode *_fno_create(struct module *owner, const char *name, struct fnode *parent);


static int _fno_fullpath(struct fnode *f, char *dst, char **p, int len)
{
    int nlen;

    if (!f)
        return -EINVAL;
    if ((f->flags & FL_LINK) == FL_LINK) {
        f =  _fno_search(f->linkname, &FNO_ROOT, 1);
    }

    if (f == &FNO_ROOT) {
        *p = dst + 1;
        dst[0] = '/';
        dst[1] = '\0';
        return 0;
    }

    if (!*p) {
        if (!f->parent)
            return -EINVAL; // what to do, how is this possible?
        _fno_fullpath(f->parent, dst, p, len);
    }

    nlen = strlen(f->fname);
    if (nlen + (*p - dst) > (len -1))
        return -ENAMETOOLONG;

    memcpy(*p, f->fname, nlen);
    *p += nlen;
    *(*p) = '/';
    *p += 1;
    *(*p + 1) = '\0';

    return 0;
}

int fno_fullpath(struct fnode *f, char *dst, int len)
{
    char *p = NULL;
    int ret;

    ret =  _fno_fullpath(f, dst, &p, len);
    if (ret == 0)  {
        int nlen = strlen(dst);
        if (nlen > 1) {
            /* Remove trailing "/" */
            dst[--nlen] = '\0';
            while (dst[nlen - 1] == '/') {
                dst[nlen - 1] = '\0';
                nlen--;
            }
        }

        return nlen;
    }

    return -ENOENT;
}

static int path_abs(char *src, char *dst, int len)
{
    struct fnode *f = &FNO_ROOT; //must fix this!

    if (src[0] == '/')
        strncpy(dst, src, len);
    else {
        if (fno_fullpath(f, dst, len) > 0) {
            while (dst[strlen(dst) - 1] == '/')
                dst[strlen(dst) - 1] = '\0';

            strncat(dst, "/", len);
            strncat(dst, src, len);

            return 0;
        }
    }

    return 0;
}

static struct fnode *fno_create_file(char *path)
{
    char *base = kalloc(strlen(path) + 1);
    struct module *owner = NULL;
    struct fnode *parent;
    struct fnode *f = NULL;
    if (!base)
        return NULL;
    basename_r(path, base);
    parent = fno_search(base);
    kfree(base);
    if (!parent)
        return NULL;
    if ((parent->flags & FL_DIR) == 0)
        return NULL;

    if (parent) {
        owner = parent->owner;
    }
    f = fno_create(owner, filename(path), parent);
    if (f)
        f->flags = 0;
    return f;
}

void fno_unlink(struct fnode *fno)
{
    struct fnode *dir;

    if (!fno)
        return;
    dir = fno->parent;

    if (fno && fno->owner && fno->owner->ops.unlink)
        fno->owner->ops.unlink(fno);

    if (!fno)
        return;

    if (dir) {
        struct fnode *child = dir->children;
        while (child) {
            if (child == fno) {
                dir->children = fno->next;
                break;
            }
            if (child->next == fno) {
                child->next = fno->next;
                break;
            }
            child = child->next;
        }
    }


    kfree(fno->fname);
    kfree(fno);
}

static struct fnode *fno_link(char *src, char *dst)
{
    struct fnode *file;
    struct fnode *link;
    int file_name_len;
    char p_src[MAX_FILE];
    char p_dst[MAX_FILE];

    path_abs(src, p_src, MAX_FILE);
    path_abs(dst, p_dst, MAX_FILE);

    file = fno_search(p_src);
    if (!file)
        return NULL;

    link = fno_create_file(p_dst);
    if (!link)
        return NULL;

    file_name_len = strlen(p_src);
    link->flags |= FL_LINK;

    link->linkname = malloc(file_name_len + 1);
    if (!link->linkname) {
        fno_unlink(link);
        return NULL;
    }

    strncpy(link->linkname, p_src, file_name_len + 1);

    return link;
}

static const char *path_walk(const char *path)
{
    const char *p = path;

    if (*p == '/') {
        while(*p == '/')
            p++;
        return p;
    }

    while ((*p != '\0') && (*p != '/'))
        p++;

    if (*p == '/')
        p++;

    if (*p == '\0')
        return NULL;

    return p;
}

/* Returns:
 * 0 = if path does not match
 * 1 = if path is in the right dir, need to walk more
 * 2 = if path is found!
 */

static int path_check(const char *path, const char *dirname)
{

    int i = 0;
    for (i = 0; dirname[i]; i++) {
        if (path[i] != dirname[i])
            return 0;
    }

    if (path[i] == '\0')
        return 2;

    if (path[i] == '/')
        return 1;

    if (i > 0 && (path[i - 1] == '/' && dirname[i - 1] == '/'))
        return 1;

    return 0;
}

static struct fnode *_fno_search(const char *path, struct fnode *dir, int follow)
{
    struct fnode *cur;
    char link[MAX_FILE];
    int check = 0;

    if (dir == NULL)
        return NULL;

    check = path_check(path, dir->fname);

    /* Does not match, try another item */
    if (check == 0) {
        if (!dir->next)
            return NULL;
        return _fno_search(path, dir->next, follow);
    }

    /* Item is found! */
    if (check == 2) {
        /* If it's a symlink, restart check */
        if (follow && ((dir->flags & FL_LINK) == FL_LINK)) {
            return _fno_search(dir->linkname, &FNO_ROOT, 1);
        }
        return dir;
    }

    /* path is correct, need to walk more */
    if( (dir->flags & FL_LINK ) == FL_LINK ){
    /* passing through a symlink */
        strcpy(link, dir->linkname);
        strcat(link, "/");
        strcat(link, path_walk(path));

        return _fno_search(link, &FNO_ROOT, follow);
    }

    return _fno_search(path_walk(path), dir->children, follow);
}

struct fnode *fno_search(const char *_path)
{
    int i, len;
    struct fnode *fno = NULL;
    char *path = NULL;

    if (!_path)
        return NULL;

    len = strlen(_path);
    if (!len)
        return NULL;

    path = calloc(len + 1, 1);
    if (!path)
        return NULL;

    memcpy(path, _path, len + 1);

    i = len - 1;
    while (i > 0) {
        if (path[i] == '/')
            path[i] = '\0';
        else
            break;
    }

    if (strlen(path) > 0) {
        fno = _fno_search(path, &FNO_ROOT, 1);
    }

    free(path);

    return fno;
}

struct fnode *fno_search_nofollow(const char *path)
{
    return _fno_search(path, &FNO_ROOT, 0);
}

static struct fnode *_fno_create(struct module *owner, const char *name, struct fnode *parent)
{
    struct fnode *fno = kcalloc(sizeof(struct fnode), 1);
    int nlen = strlen(name);
    if (!fno)
        return NULL;

    fno->fname = kalloc(nlen + 1);
    if (!fno->fname){
        kfree(fno);
        return NULL;
    }

    memcpy(fno->fname, name, nlen + 1);
    if (!parent) {
        parent = &FNO_ROOT;
    }

    fno->parent = parent;
    fno->next = fno->parent->children;
    fno->parent->children = fno;

    fno->children = NULL;
    fno->owner = owner;
    return fno;
}

struct fnode *fno_create(struct module *owner, const char *name, struct fnode *parent)
{
    struct fnode *fno = _fno_create(owner, name, parent);
    if (fno && parent && parent->owner && parent->owner->ops.creat)
        parent->owner->ops.creat(fno);
    fno->flags |= FL_RDWR;
    return fno;
}

static void mkdir_links(struct fnode *fno)
{
    char path[MAX_FILE], selfl[MAX_FILE], parentl[MAX_FILE], path_basename[MAX_FILE];

    fno_fullpath(fno, path, MAX_FILE - 4);
    strcpy(selfl, path);
    strcpy(parentl, path);
    strcat(selfl, "/.");
    strcat(parentl, "/..");

    if (fno) {
        fno_link(path, selfl);
        basename_r(path, path_basename);
        fno_link(path_basename, parentl);
    }
}

struct fnode *fno_mkdir(struct module *owner, const char *name, struct fnode *parent)
{
    struct fnode *fno = _fno_create(owner, name, parent);
    fno->flags |= (FL_DIR | FL_RDWR);
//    if (parent && parent->owner && parent->owner->ops.creat)
//        parent->owner->ops.creat(fno);
    mkdir_links(fno);
    return fno;
}

int vfs_unlink(void *arg1)
{
    char *path = (char *)arg1;
    printf("Trying to rm %s\n", path);
    char abs_p[MAX_FILE];
    struct fnode *f;
//    if (task_ptr_valid(path))
//        return -EACCES;
    path_abs(path, abs_p, MAX_FILE);
    f = fno_search(path); /* Don't follow symlink */
    if (f) {
        fno_unlink(f);
        return 0;
    }
    return -ENOENT;
}


struct fnode *vfs_opendir(void *arg1)
{
    struct fnode *fno;
    fno = fno_search((char *)arg1);

    if (fno && (fno->flags & FL_DIR)) {
        if (fno->flags & FL_INUSE)
            return NULL; /* XXX EBUSY */
        /* Use .off to store current readdir ptr */
        fno->off = (int)fno->children;
        fno->flags |= FL_INUSE;
        return fno;
    } else {
        return NULL;
    }
}

int vfs_readdir(void *arg1, void * arg2, void * arg3)
{
    struct fnode *fno;
    struct fnode *next;
    struct dirent *ep;
    struct dirent **res;

    fno = (struct fnode *)arg1;
    ep = (struct dirent *)arg2;
    res = (struct dirent **)arg3;
    next = (struct fnode *)fno->off;

    if (!fno || !ep)
        return -ENOENT;

    if (!next) {
        return -1;
    }

    fno->off = (int)next->next;
    ep->d_ino = 0; /* TODO: populate with inode? */
    strncpy(ep->d_name, next->fname, 256);
    *res = ep;

    return 0;
}

int vfs_closedir(void *arg1)
{
    struct fnode *fno = (struct fnode *)arg1;

    fno->off = 0;
    fno->flags &= ~(FL_INUSE);

    return 0;
}

int vfs_stat(char *path, struct stat *st)
{
    char abs_p[MAX_FILE];
    struct fnode *fno;

    path_abs(path, abs_p, MAX_FILE);
    fno = _fno_search(abs_p, &FNO_ROOT, 0);

    if (!fno) {
        return -ENOENT;
    }

    if (fno->flags & FL_DIR) {
        st->st_mode = S_IFDIR;
        st->st_size = 0;
    } else if (fno->flags & FL_LINK) {
        return vfs_stat(fno->linkname, st); /* Stat follows symlink */
    } else {
        st->st_mode = S_IFREG;
        st->st_size = fno->size;
    }

    if (fno->flags & FL_EXEC) {
        st->st_mode |= P_EXEC;
    }

    return 0;
}

int vfs_mount(char *source, char *target, char *module, uint32_t flags, void *args)
{
    struct module *m;
    if (!module || !target)
        return -ENOMEDIUM;
    m = module_search(module);
    if (!m || !m->mount)
        return -EOPNOTSUPP;
    if (m->mount(source, target, flags, args) == 0) {
        struct mountpoint *mp = kalloc(sizeof(struct mountpoint));
        if (mp) {
            mp->target = fno_search(target);
            mp->next = MTAB;
            MTAB = mp;
        }
        return 0;
    }
    return -ENOENT;
}

/*
int vfs_mount(char *source, char *target, char *module, uint32_t flags, void *args)
{
    fatfs_mount(source, target, flags, args);
    struct mountpoint *mp = malloc(sizeof(struct mountpoint));

    if (mp) {
        mp->target = fno_search(target);
        mp->next = MTAB;
        MTAB = mp;
    }

    return 0;
}
*/

struct fnode *vfs_open(void *arg1, uint32_t arg2)
{
    char *rel_path = (char *)arg1;
    struct fnode *f;
    uint32_t flags = arg2;
    char path[MAX_FILE];
    char ppath[MAX_FILE];
    int ret;

    path_abs(rel_path, path, MAX_FILE);

    f = fno_search(path);
    if (f) {
        if (fatfs_open(path, flags))
            return f;
    }

    f = fno_create_file(path);
    if (f) {
        if (fatfs_open(path, flags))
            return f;
    }

    return NULL;
}

int vfs_read(struct fnode *fno, void *buf, int len)
{
    return fatfs_read(fno, buf, len);
}

int vfs_write(struct fnode *fno, void *buf, int len)
{
    return fatfs_write(fno, buf, len);
}

int vfs_seek(struct fnode *fno, int off, int whence)
{
    return fatfs_seek(fno, off, whence);
}

/* Frosted device driver hook */
static struct module mod_sdio;
struct dev_sd {
    struct device * dev;
};

struct dev_sd SdCard[1]; /* Multiple slots? Make this array bigger */

void vfs_init(void)
{
    struct fnode *dev = NULL;
    /* Initialize "/" */
    FNO_ROOT.owner = NULL;
    FNO_ROOT.fname = "/";
    FNO_ROOT.parent = &FNO_ROOT;
    FNO_ROOT.children = NULL;
    FNO_ROOT.next = NULL ;
    FNO_ROOT.flags = FL_DIR | FL_RDWR;

    /* Init "/dev" dir */
    dev = fno_mkdir(NULL, "dev", NULL);
    char name[4] = "sd0";
    mod_sdio.ops.block_read = mb_read;
    mod_sdio.ops.block_write = mb_write;
    dev = fno_create(&mod_sdio, name, dev);
    dev->flags |= FL_BLK;

//    memset(&SdCard[0], 0, sizeof(struct dev_sd));
    printf("Found SD card in microSD slot.\r\n");
//    SdCard[0].dev = device_fno_init(&mod_sdio, name, dev, FL_BLK, &SdCard[0]);

    /* Init "/sys" dir */
    dev = fno_mkdir(NULL, "sys", NULL);

    /* Init "/tmp" dir */
    dev = fno_mkdir(NULL, "tmp", NULL);

    /* Init "/bin" dir */
    dev = fno_mkdir(NULL, "bin", NULL);

    /* Init "/mnt" dir */
    dev = fno_mkdir(NULL, "mnt", NULL);

    fatfs_init();
}


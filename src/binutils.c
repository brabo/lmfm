/*
 *      binutils.c: basic binutils for lmfm
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
 *      Authors: brabo
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include "fat.h"
#include "vfs.h"

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
    uint32_t    nclusts;
    uint8_t     mounted;
};

int mount(struct fatfs *fs)
{
    printf("\nC:\\MOUNT /dev/sd0 /mnt\n");

    if (!fs)
        return -1;

    vfs_mount("/dev/sd0", "/mnt", "fatfs", 0, NULL);

    return 0;
}

int ls(char *path)
{
    printf("\nC:\\LS %s\n", path);

    char *fname;
    char *fpath;
    struct dirent *ep, *result;
    void *d;
    struct stat st;
    char type;
    int i;
    char ch_size[8] = "";
    char mag[] = " KMGT";

    fname = malloc(MAX_FILE);
    fpath = malloc(MAX_FILE);

    ep = malloc(sizeof(struct dirent));
    if (!ep || !fname || !fpath)
        return -1;

    d = vfs_opendir(path); // FIXME: DIR* vs. struct fno *
    if (!d) {
        fprintf(stderr, "Error opening %s\r\n", path);
        return -2;
    }

    while(vfs_readdir(d, ep, &result) == 0) {

        fname[0] = '\0';
        strncat(fname, ep->d_name, MAX_FILE);
        fpath[0] = '\0';
        strncat(fpath, path, MAX_FILE);
        strncat(fpath, fname, MAX_FILE);

        if (vfs_stat(fpath, &st) == 0) {
            printf(fname);
                printf( "\t");
                if (S_ISDIR(st.st_mode)) {
                    type = 'd';
                } else if (S_ISLNK(st.st_mode)) {
                    type = 'l';
                } else {
                    unsigned long size = st.st_size;;
                    int order = 0;
                    char magn;
                        while (size > 1000) {
                            size /= 1000;
                            order++;
                        }
                        magn = mag[order];

                    snprintf(ch_size, 9, "%lu%c ", size, magn);
                    type = 'f';
                }

                printf( "%c", type);
                printf( "    ");
                printf( ch_size);
            printf( "\r\n");
        } else {
            fprintf(stderr, "stat error on %s: %s.\r\n", fname, strerror(errno));
        }
    }
    vfs_closedir(d);
    free(ep);
    free(fname);
    free(fpath);

    return 0;
}

void cat(char *path)
{
    printf("\nC:\\CAT %s\n", path);

    struct fnode *fno = vfs_open(path, O_RDONLY);

    if (!fno)
        return;

    uint8_t *buf = malloc((fno->size + 1) * sizeof (uint8_t));
    memset(buf, 0, (fno->size + 1));
    int res = vfs_read(fno, buf, fno->size);

    if (!res)
        return;
    while (2 > 1) {
        if (!*buf)
            break;
        putchar(*buf++);
    }

    fatfs_close(fno);
}

void fuzz(char *path)
{
    printf("\nC:\\FUZZ %s\n", path);

    struct fnode *fno = vfs_open(path, O_RDONLY);

    srand(time(NULL));
    int i, n, o;

    for (i = 0; i < 100; i++) {
        n = rand() % fno->size;
        o = rand() % fno->size;

        uint8_t *buf = malloc((n + 1) * sizeof (uint8_t));
        memset(buf, 0, (n + 1));

        int res = vfs_seek(fno, o, SEEK_SET);
        if (res != o) {
            printf("OOPS..\n");
            continue;
        }

        res = vfs_read(fno, buf, n);
        //print_array(buf, n);
    }

    printf("If you see me, we did not crash!\n\n");

    fatfs_close(fno);
}

int touch(char *path)
{
    if (!path)
        return -1;

    printf("C:\\TOUCH %s\n", path);

    struct fnode *fno = vfs_open(path, O_RDONLY);
    if (fno)
        fatfs_close(fno);
    else
        printf("NO FNO!\n");

    return 0;
}

int mk_file(char *path, uint8_t *buf, int len)
{
    if (!path)
        return -1;

    printf("C:\\CP MEM %s\n", path);

    struct fnode *fno = vfs_open(path, O_RDONLY);
    if (!fno)
        return -2;

    int res = vfs_write(fno, buf, len);

    fatfs_close(fno);

    return res;
}

int edit_file(char *path)
{
    if (!path)
        return -1;

    printf("C:\\VI %s\n", path);

    struct fnode *fno = vfs_open(path, O_RDWR);

    if (!fno)
        return -2;

    uint8_t buf[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
    uint8_t buf2[4] = { 0x42, 0x42, 0x42, 0x42 };

    int res;
    res = vfs_seek(fno, (5096 + 256), SEEK_SET);
    res = vfs_write(fno, buf, 4);
    res = vfs_seek(fno, (5096 + 256), SEEK_SET);
    res = vfs_read(fno, buf2, 4);

    printf("Found 0x%02X%02X%02X%02X\n", buf2[0], buf2[1], buf2[2], buf2[3]);

    fatfs_close(fno);

    return res;
}

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
#include "fat.h"
#include "vfs.h"


/*
int mount(struct fatfs *fs)
{
    if (!fs)
        return -1;

    return fat_mount(fs);
}

int ls(struct fatfs *fs, char *path)
{
    if (!fs || !path)
        return -1;

    printf("C:\\LS %s\n", path);

    struct fatfs_dir dj;
    char fname[13];
    dj.fn = fname;
    memset(fname, 0x00, 13);

    fat_open(fs, &dj, path);

    while (!fat_readdir(fs, &dj)) {
        printf("%s\n", dj.fn);
    }
    printf("\n");

    return 0;
}

int cat(struct fatfs *fs, char *path)
{
    if (!fs || !path)
        return -1;

    printf("C:\\CAT %s\n", path);

    struct fatfs_dir dj;
    char fname[13];
    memset(fname, 0x00, 13);
    dj.fn = fname;

    int res = fat_open(fs, &dj, path);

    uint8_t *buf = malloc((dj.fsize + 1) * sizeof (uint8_t));
    memset(buf, 0, (dj.fsize + 1));
    res = fat_read(fs, &dj, buf, dj.fsize);

    while (2 > 1) {
        if (!*buf)
            break;
        putchar(*buf++);
    }
    printf("\n\n");

    return 0;
}

int touch(struct fatfs *fs, char *path)
{
    if (!fs || !path)
        return -1;

    printf("C:\\TOUCH %s\n", path);

    struct fatfs_dir dj;
    char fname[13];
    memset(fname, 0x00, 13);
    dj.fn = fname;

    int res = fat_open(fs, &dj, path);

    return res;
}

int mk_file(struct fatfs *fs, char *path, uint8_t *buf, int len)
{
    if (!fs || !path)
        return -1;

    printf("C:\\CP MEM %s\n", path);

    struct fatfs_dir dj;
    char fname[13];
    memset(fname, 0x00, 13);
    dj.fn = fname;

    int res = fat_open(fs, &dj, path);
    if (res)
        return res;

    res = fat_write(fs, &dj, buf, len);

    return res;
}

int edit_file(struct fatfs *fs, char *path)
{
    if (!fs || !path)
        return -1;

    printf("C:\\VI %s\n", path);

    struct fatfs_dir dj;
    char fname[13];
    memset(fname, 0x00, 13);
    dj.fn = fname;

    int res = fat_open(fs, &dj, path);
    if (res)
        return res;

    uint8_t buf[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
    uint8_t buf2[4] = { 0x42, 0x42, 0x42, 0x42 };

    res = fat_lseek(fs, &dj, (5096 + 256));
    res = fat_write(fs, &dj, buf, 4);
    res = fat_lseek(fs, &dj, (5096 + 256));
    res = fat_read(fs, &dj, buf2, 4);
    printf("Found 0x%02X%02X%02X%02X\n", buf2[0], buf2[1], buf2[2], buf2[3]);

    return res;
}
*/
int vfs_mount_fat(struct fatfs *fs)
{
    if (!fs)
        return -1;
    vfs_mount("/dev/sd0", "/mnt", "fatfs", 0, NULL);
    return 0;
}

int vfs_ls(char *path)
{
    char *fname;
    char *fpath;
    struct dirent *ep, *result;
    DIR *d;
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

    d = (DIR *)vfs_opendir(path);
    if (!d) {
        fprintf(stderr, "Error opening %s\r\n", path);
        return -2;
    }
    printf("C:\\LS %s\n", path);
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
    printf("\n");
    vfs_closedir(d);
    free(ep);
    free(fname);
    free(fpath);

    return 0;
}

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
#include "fat.h"

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
    char fname[12];
    dj.fn = fname;

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
    char fname[12];
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
    printf("\n");

    return 0;
}

int touch(struct fatfs *fs, char *path)
{
    if (!fs || !path)
        return -1;

    printf("C:\\TOUCH %s\n", path);

    struct fatfs_dir dj;
    char fname[12];
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
    char fname[12];
    dj.fn = fname;

    int res = fat_open(fs, &dj, path);
    if (res)
        return res;

    res = fat_write(fs, &dj, buf, len);

    return res;
}

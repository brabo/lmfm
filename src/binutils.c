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

int ls(struct fatfs *fs, char *path)
{
    if (!fs || !path)
        return -1;

    printf("Reading directory: %s\n", path);

    struct fatfs_dir dj;
    char fname[12];
    dj.fn = fname;

    fat_open(fs, &dj, path);
    printf("Opened directory at: 0x%08X\n", dj.sect * 512);

    while (!fat_readdir(fs, &dj)) {
        printf("%s\n", dj.fn);
    }

    return 0;
}

int cat(struct fatfs *fs, char *path)
{
    if (!fs || !path)
        return -1;

    printf("Reading file: %s\n", path);

    struct fatfs_dir dj;
    char fname[12];
    dj.fn = fname;

    int res = fat_open(fs, &dj, path);

    printf("Found file %s with %d bytes length\n", dj.fn, dj.fsize);

    uint8_t *buf = malloc((dj.fsize + 1) * sizeof (uint8_t));
    memset(buf, 0, (dj.fsize + 1));
    res = fat_read(fs, &dj, buf, dj.fsize);

    printf("Buffer %d bytes:\n", res);

    while (2 > 1) {
        if (!*buf)
            break;
        putchar(*buf++);
    }

    return 0;
}

/*
 *      mockblock.c: a linux userspace mock block driver to operate on
 *                      dd images files for developing fatfs driver.
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
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

/* prototypes */
int mb_read(void *fno, void *_buf, uint32_t lba, int offset, int count);
int mb_write(void *fno, void *_buf, uint32_t lba, int offset, int count);
int mb_init(char *mb_name);
int mb_close(void);

int fd;

int mb_read(void *fno, void *_buf, uint32_t lba, int offset, int count)
{
    off_t off;
    int len;

    //printf("sector %d\noffset %d\n", lba, offset);
    off = lseek(fd, (off_t)((lba * 512) + offset), SEEK_SET);
    if (off < 0) {
        perror("mb_read: lseek: ");
        return -1;
    }

    if (off != ((lba * 512) + offset)) {
        return -2;
    }

    if (count + offset > 512) {
        count = 512 - offset;
    }

    len = read(fd, _buf, count);
    if (len < 0) {
        perror("mb_read: read: ");
        return -3;
    }

    return len;
}

int mb_write(void *fno, void *_buf, uint32_t lba, int offset, int count)
{
    off_t off;
    int len;

    off = lseek(fd, (off_t)((lba * 512) + offset), SEEK_SET);
    if (off < 0) {
        perror("mb_write: lseek: ");
        return -1;
    }

    if (off != ((lba * 512) + offset)) {
        return -2;
    }

    if (count + offset > 512) {
        count = 512 - offset;
    }

    len = write(fd, _buf, count);
    if (len < 0) {
        perror("mb_write: write: ");
        return -3;
    }

    return len;
}

int mb_init(char *mb_name)
{
    if (!mb_name)
        return -1;

    fd = open(mb_name, O_RDWR);
    if (fd == -1) {
        perror("mb_init: open: ");
        return -2;
    }

    return fd;
}

int mb_close(void)
{
    close(fd);

    return 0;
}

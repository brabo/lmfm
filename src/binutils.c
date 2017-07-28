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
#include "fat.h"

int ls(struct fatfs *fs, char *path)
{
    if (!fs || !path)
        return -1;

    printf("trying to read directory %s\n", path);

    struct fatfs_dir dj;
    char fname[12];
    dj.fn = fname;


    open_dir(fs, &dj, path);
    printf("trying to read directory at 0x%08X\n", dj.sect * 512);

    read_dir(fs, &dj);

    return 0;
}

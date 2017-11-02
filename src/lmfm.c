/*
 *      Lean Mean FAT Machine
 *
 *      This is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as
 *      published by the Free Software Foundation.
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
#include "vfs.h"
#include "mockblock.h"

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

#include "binutils.h"

int main(int argc, char **argv)
{
    struct fatfs *fs = malloc(1 * sizeof (struct fatfs));
    fs->bsect = 0;

    if (argc != 2) {
        printf("Usage: %s image\n", argv[0]);
        exit(1);
    }


    fs->fd = mb_init(argv[1]);

    if (fs->fd < 0)
        exit(2);

    vfs_init();
    ls("/");
    ls("/mnt/");

    mount(fs);

    ls("/");
    ls("/mnt/");
    ls("/mnt/BARDIR/");

    //char *buf = "GGRR: the quick brown fox jumps\nover the lazy dog!!!\n";
    //uint8_t *buf = malloc(14096 * sizeof (uint8_t));
    //memset(buf, 'Z', 14096);
    //mk_file("/mnt/GRR", buf, 14096);
    //edit_file("/mnt/ZRR");
    cat("/mnt/GRR");

    mb_close();

    free(fs);

    // if you're happy and you know it,
    exit(0);
}

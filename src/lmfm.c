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
#include "mockblock.h"
#include "binutils.h"

int main(int argc, char **argv)
{
    struct fatfs *fs = malloc(1 * sizeof (struct fatfs));
    fs->bsect = 0;

    if (argc != 2)
        exit(1);

    fs->fd = mb_init(argv[1]);

    if (fs->fd < 0)
        exit(2);

    mount(fs);

    ls(fs, "/");

    /* uncomment next two lines to make a file with content */
    //char *buf = "GGRR: the quick brown fox jumps\nover the lazy dog!!!\n";
    //uint8_t *buf = malloc(10240 * sizeof (uint8_t));
    //memset(buf, 'Z', 10240);
    //mk_file(fs, "/GRR", buf, 10240);

    ls(fs, "/");

    /* these would have to exist, but can be created by mk_file above */
    cat(fs, "/ERR");
    cat(fs, "/GRR");
    cat(fs, "/FRR");
    cat(fs, "/ZRR");
    cat(fs, "/AARR");
    cat(fs, "/BBRR");
    cat(fs, "/GGRR");
    cat(fs, "/ZZRR");

    mb_close(fs->fd);

    free(fs);
    // if you're happy and you know it,
    exit(0);
}

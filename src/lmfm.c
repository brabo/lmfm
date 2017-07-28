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
#include "fat.h"
#include "mockblock.h"

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

    uint32_t fat = get_fat(fs, 2);
    printf("FAT 2: 0x%08X\n", fat);
    fat = get_fat(fs, 0x0b);
    printf("FAT 0x0b: 0x%08X\n", fat);
    uint32_t next = walk_fat(fs);
    fat = get_fat(fs, next);
    printf("FAT 0x%08X: 0x%08X\n", (fs->fatbase * fs->bps) + (next * 4), fat);

    // if you're happy and you know it,
    exit(0);
}

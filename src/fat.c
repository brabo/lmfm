/*
 *      fat.c: a linux userspace fatfs driver to operate on
 *                      dd images files through mockblock.
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
#include "mockblock.h"

#define BS_JMPBOOT      0
#define BPB_BPS         11
#define BPB_SPC         13
#define BPB_RSVD_SEC    14
#define BPB_NUMFATS     16
#define BPB_ROOTENTS    17
#define BPB_TOTSEC16    19
#define BPB_FATSz16     22
#define BPB_TOTSEC32    32
#define BPB_FATSz32     36
#define BPB_ROOTCLUS    44
#define BS_FSTYPE       54
#define BS_FSTYPE32     82
#define BS_MBR          446
#define BS_55AA         510

#define DIR_NAME        0
#define DIR_ATTR        11
#define DIR_SCLUST_HI   20
#define DIR_SCLUST_LO   26
#define DIR_FSIZE       28



#define FAT12   1
#define FAT16   2
#define FAT32   3
#define EOC32   0x0FFFFFF8

#define LD_WORD(ptr)        (uint16_t)(*(uint16_t *)(ptr))
#define LD_DWORD(ptr)       (uint32_t)(*(uint32_t *)(ptr))

int print_array(uint8_t *buf, int len)
{
    char *bugger = malloc(sizeof(uint8_t) * ((len * 2) + 3));
    char *ptr = bugger;
    uint8_t *ptr2 = buf;
    sprintf(ptr++, "[");

    for (int i = 0; i < len; i++) {
        sprintf(ptr, "%02X", *ptr2++);
        ptr += 2;
    }

    sprintf(ptr++, "]");
    printf("Read sector: %s\n", bugger);

    free(bugger);

    return 0;
}

static int check_fs(struct fatfs *fs)
{
    if (LD_WORD(fs->win + BS_55AA) != 0xAA55) {
        return -1;
    }

    printf("Boot signature found.. going on..\n");

    if (fs->win[BS_JMPBOOT] == 0xE9 || (fs->win[BS_JMPBOOT] == 0xEB && fs->win[BS_JMPBOOT + 2] == 0x90)) {
        printf("Found valid jmpBoot.\n");

        if ((LD_WORD(fs->win + BS_FSTYPE)) == 0x4146) return 0;   /* Check "FAT" string */
        if ((LD_WORD(fs->win + BS_FSTYPE32) == 0x4146) && (LD_WORD(fs->win + BS_FSTYPE32 + 2) == 0x3354) && (*(fs->win + BS_FSTYPE32 + 4) == 0x32)) {
            fs->type = FAT32;
            printf("Found FAT32 string.\n");
            return 0;            /* Check "FAT3" string */
        }
    }

    return -2;
}

int get_fat(struct fatfs *fs, int clust)
{

    if (clust < 2 || clust >= fs->n_fatent) { /* Range check */
        return -1;
    }

    switch(fs->type) {
    case FAT12:
        break;
    case FAT16:
        if (mb_read(fs->fd, fs->win, 0, (fs->fatbase + (clust / (fs->bps / 2))) * fs->bps, fs->bps)) break;
        return LD_WORD(fs->win + (clust * 2));
    case FAT32:
        if (!mb_read(fs->fd, fs->win, 0, (fs->fatbase + (clust / (fs->bps / 4))) * fs->bps, fs->bps)) break;

        return (LD_DWORD(fs->win + ((clust * 4) % fs->bps)) & 0x0FFFFFFF);
    }
    return -2;
}

static int clust2sect(struct fatfs *fs, uint32_t clust)
{
    clust -= 2;
    return ((clust * fs->spc + fs->database));
}

static int get_clust(struct fatfs *fs, uint8_t* dir)
{
    int clst = 0;

    if (fs->type == FAT32) {
        clst = LD_WORD(dir + DIR_SCLUST_HI);
        clst <<= 16;
    }
    clst |= LD_WORD(dir + DIR_SCLUST_LO);

    return clst;
}


int mount(struct fatfs *fs)
{
    mb_read(fs->fd, fs->win, 0, (fs->bsect * 512), 512);
    print_array(fs->win, 512);

    if (check_fs(fs) == -2) {
        fs->bsect = LD_WORD(fs->win + BS_MBR + 8);
        mb_read(fs->fd, fs->win, 0, (fs->bsect * 512), 512);
        print_array(fs->win, 512);
        if (check_fs(fs) < 0) {
            printf("Cannot find any valid boot sector, quitting..\n");
            exit(3);
        }
        if (fs->type == FAT32) {
        }
    }
    printf("Boot sector: %d\n", fs->bsect);

    fs->bps = LD_WORD(fs->win + BPB_BPS);
    printf("Bytes per sector: %d\n", fs->bps);
    fs->spc = fs->win[BPB_SPC];
    printf("Sectors per cluster: %d\n", fs->spc);
    uint16_t root_ents = LD_WORD(fs->win + BPB_ROOTENTS);
    printf("Root entries: %d\n", root_ents);
    uint32_t root_secs;
    root_secs = ((root_ents * 32) + (fs->bps -1)) / fs->bps;
    printf("Root sectors: %d\n", root_secs);
    uint32_t fatsz;
    fatsz = LD_WORD(fs->win + BPB_FATSz16);
    if (!fatsz)
        fatsz = LD_DWORD(fs->win + BPB_FATSz32);

    printf("FAT size: %d\n", fatsz);
    uint8_t num_fats = fs->win[BPB_NUMFATS];
    printf("Number of FATs: %d\n", num_fats);
    uint16_t rsvd_sec;
    rsvd_sec = LD_WORD(fs->win + BPB_RSVD_SEC);
    printf("Reserved sectors: %d\n", rsvd_sec);
    fs->database = rsvd_sec + (num_fats * fatsz) + root_secs;
    printf("Data sector base relative to boot sector: %d\n", fs->database);
    fs->database += fs->bsect;
    printf("Data sector base: %d\n", fs->database);
    uint32_t totsec;
    totsec = LD_WORD(fs->win + BPB_TOTSEC16);
    if (!totsec)
        totsec = LD_DWORD(fs->win + BPB_TOTSEC32);
    printf("Total sectors: %d\n", totsec);
    uint32_t datasec;
    datasec = totsec - (rsvd_sec + (num_fats * fatsz) + root_secs);
    printf("Data sectors: %d\n", datasec);
    uint32_t nclusts;
    nclusts = datasec / fs->spc;
    printf("Total clusters: %d\n", nclusts);
    fs->fatbase = rsvd_sec + fs->bsect;
    printf("FAT base: %d\n", fs->fatbase);
    fs->dirbase = clust2sect(fs, LD_DWORD(fs->win + BPB_ROOTCLUS));
    printf("Root dir start: 0x%08X\n", fs->dirbase * 512);

    // fat type determination

    if (nclusts < 4085) {
        fs->type = FAT12;
        printf("Reality check: is FAT12\n");
    } else if (nclusts < 65525) {
        fs->type = FAT16;
        printf("Reality check: is FAT16\n");
    } else {
        fs->type = FAT32;
        printf("Reality check: is FAT32\n");
    }

    fs->n_fatent = nclusts + 2;

    return 0;
}

int walk_fat(struct fatfs *fs)
{
    int fat;
    int clust = 2;
    while (2 > 1) {
        fat = get_fat(fs, clust);
        if (!fat)
            break;
        if ((0x00000001 < fat) && (fat < 0x0FFFFFF0))
            printf("Found FAT entry: 0x%08X\n", fat);
        else if (fat > 0x0FFFFFF7)
            printf("Found EOC FAT entry: 0x%08X\n", fat);
        else
            printf("Found special FAT entry: 0x%08X\n", fat);
        clust++;
    }

    return clust;
}

static int dir_rewind(struct fatfs *fs, struct fatfs_dir *dj)
{
    if (dj->cclust == 1 || dj->cclust >= fs->n_fatent)
        return -1;

    if (!dj->cclust) {
        dj->sect = fs->dirbase;
    } else {
        dj->sect = clust2sect(fs, dj->cclust);
    }

    return 0;
}

static int dir_read(struct fatfs *fs, struct fatfs_dir *dj)
{
    if (!fs || !dj)
        return -1;

    mb_read(fs->fd, fs->win, 0, (dj->sect * fs->bps), fs->bps);
    print_array(fs->win, fs->bps);

    while (2 > 1) {
        uint8_t *off = fs->win + dj->off;
        if (!*off) /* Free FAT entry, no more FAT entries! */
            return -2;
        if (*(off + DIR_ATTR) & 0x0F) { /* LFN entry */
            dj->off += 32;
            continue;
        } else {
            int i;
            for (i = 0; i < 8; i++) {   /* Copy file name body */
                dj->fn[i] = off[i];
                if (dj->fn[i] == ' ') break;
            }
            dj->fn[i] = 0x00;
            dj->attr = *(off + DIR_ATTR);
            dj->sclust = get_clust(fs, off);
            dj->fsize = LD_DWORD(off + DIR_FSIZE);
            dj->off += 32;
            break;
        }
    }

    return 0;
}

static int dir_find(struct fatfs *fs, struct fatfs_dir *dj, char *path)
{
    if (!fs || !dj || !path)
        return -1;

    while (dir_read(fs, dj) == 0) {
        if (!strncmp(dj->fn, path, 11)) {
            dj->off -= 32;
            uint32_t fat = get_fat(fs, dj->sclust);
            dj->sect = clust2sect(fs, dj->sclust);
            dj->cclust = dj->sclust;
            return 0;
        }
    }

    return -2;
}

static int follow_path(struct fatfs *fs, struct fatfs_dir *dj, char *path)
{
    int res;

    while (*path == ' ')
        path++;

    if (*path == '/')
        path++;

    dj->cclust = 0;

    res = dir_rewind(fs, dj);
    if (*path < ' ') {
        return res;
    }

    dj->off = 0;

    do {
        char tpath[12];
        char *tpathp = tpath;

        while ((*path != '/') && (*path != ' ') && (*path != 0x00)) {
            *tpathp++ = *path++;

        }
        *tpathp = 0x00;
        dir_find(fs, dj, tpath);
        if (*path == '/')
            path++;
    } while ((*path != ' ') && (*path != 0x00));


    return 0;
}

int fat_open(struct fatfs *fs, struct fatfs_dir *dj, char *path)
{
    if (!fs || !dj || !path)
        return -1;

    int ret = follow_path(fs, dj, path);
    if (ret)
        return ret;

    dj->off = 0;

    return 0;
}

int fat_readdir(struct fatfs *fs, struct fatfs_dir *dj)
{
    if (!fs || !dj)
        return -1;

    int res = dir_read(fs, dj);
    if (res)
        return -1;

    return 0;
}

int fat_read(struct fatfs *fs, struct fatfs_dir *dj, uint8_t *buf, int len)
{
    if (!fs || !dj)
        return -1;

    int r_len = 0, sect = 0, off;

    while (r_len < len) {
        mb_read(fs->fd, fs->win, 0, ((dj->sect + sect) * fs->bps), fs->bps);
        print_array(fs->win, fs->bps);

        for (off = 0; r_len < dj->fsize && r_len < len && off < fs->bps; r_len++) {
            memcpy(buf++, (fs->win + off++), 1);
        }
        off = 0;
        sect++;
        if ((sect + 1) > fs->spc) {
            dj->cclust = get_fat(fs, dj->cclust);
            dj->sect = clust2sect(fs, dj->cclust);
            sect = 0;
        }
    }

    return r_len;
}

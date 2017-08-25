/*
 *      frag.c: file fragmenter for lmfm
 *              we need to fragment files to test..
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
#include <errno.h>
#include <time.h>
#include "mockblock.h"
#include "fat.h"

/* Macro proxies for disk operations */
#define disk_read(f,b,s,o,l) mb_read(f->blockdev,b,s,o,l)
#define disk_write(f,b,s,o,l) mb_write(f->blockdev,b,s,o,l)


#define CLUST2SECT(f, c) (((c - 2) * f->spc + f->database))


static int move_cluster(struct fatfs_disk *fsd, uint32_t dst, uint32_t src)
{
    if (!fsd)
        return -EINVAL;

    struct fatfs *fs = fsd->fs;
    uint8_t buf[fs->bps];
    uint32_t ssect = CLUST2SECT(fs, src);
    uint32_t dsect = CLUST2SECT(fs, dst);
    int i;

    for (i = 0; i < fs->spc; i++) {
        disk_read(fsd, buf, (ssect + i), 0, fs->bps);
        disk_write(fsd, buf, (dsect + i), 0, fs->bps);
        memset(buf, 0, 512);
        disk_write(fsd, buf, (ssect + i), 0, fs->bps);
    }

    return 0;
}

static uint32_t get_random_freefat(struct fatfs_disk *fsd)
{
    srand(time(NULL));

    while (2 > 1) {
        uint32_t tmpclust = rand() % fsd->fs->nclusts;
        uint32_t tmpfat = get_fat(fsd, tmpclust);
        if (!tmpfat)
            return tmpclust;
    }
}

int frag(char *path)
{
    if (!path)
        return -EINVAL;
    struct fnode *fno = fno_search(path);

    if (!fno || !fno->priv)
        return -EINVAL;

    struct fatfs_priv *priv = (struct fatfs_priv *)fno->priv;
    struct fatfs_disk *fsd = priv->fsd;
    struct fatfs *fs = fsd->fs;
    int i;
    uint32_t nclust, tmpclust;
    nclust = fno->size / (fs->spc * fs->bps);

    for (i = 0; i < nclust; i++) {
        uint32_t src = priv->fat[i];
        priv->fat[i] = get_random_freefat(fsd);
        move_cluster(fsd, priv->fat[i], tmpclust);
    }

    priv->sclust = priv->cclust = priv->fat[0];
    // directory entry needs to be set, we will need to cache right sector
    // and right offset
    disk_read(fsd, fs->win, priv->dirsect, 0, fs->bps);
    set_clust(fs, (fs->win + priv->off), priv->sclust);
    disk_write(fsd, fs->win, priv->dirsect, 0, fs->bps);

    for (i = 0; (i < (nclust - 1)); i++) {
        set_fat(fsd, priv->fat[i], priv->fat[i + 1]);
    }
}

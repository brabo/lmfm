#ifndef __FAT_H
#define __FAT_H

struct bpb {
    uint16_t    bps;
    uint8_t     spc;
};

struct fatfs {
    int         fd;
    uint8_t     win[512];
    uint32_t    bsect;
    uint8_t     type;
    struct bpb;
    uint32_t    database;
    uint32_t    fatbase;
    uint32_t    dirbase;
    uint32_t    n_fatent;
};

struct fatfs_dir {
    uint8_t     *fn;
    uint32_t    clust;
    uint32_t    sect;
};

int get_fat(struct fatfs *fs, int clust);
int mount(struct fatfs *fs);
int walk_fat(struct fatfs *fs);
int open_dir(struct fatfs *fs, struct fatfs_dir *dj, char *path);
int read_dir(struct fatfs *fs, struct fatfs_dir *dj);

#endif

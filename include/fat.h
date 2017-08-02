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
    uint32_t    sclust;
    uint32_t    cclust;
    uint32_t    sect;
    uint32_t    off;
    uint8_t     attr;
    uint32_t    fsize;
    uint32_t    dirsect;
    uint32_t    foff;
};

int print_array(uint8_t *buf, int len);
int fat_mount(struct fatfs *fs);
int fat_create(struct fatfs *fs, struct fatfs_dir *dj, char *path);
int fat_open(struct fatfs *fs, struct fatfs_dir *dj, char *path);
int fat_readdir(struct fatfs *fs, struct fatfs_dir *dj);
int fat_read(struct fatfs *fs, struct fatfs_dir *dj, uint8_t *buf, int len);
int fat_write(struct fatfs *fs, struct fatfs_dir *dj, uint8_t *buf, int len);
int fat_lseek(struct fatfs *fs, struct fatfs_dir *dj, uint32_t offset);

#endif

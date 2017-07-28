#ifndef __FAT_H
#define __FAT_H

struct bpb {
    uint16_t    bps;
    uint8_t     spc;
    uint16_t    rsvd_sec;
    uint8_t     num_fats;
    uint8_t     root_ents;
};

struct fatfs {
    int         fd;
    uint8_t     win[512];
    uint32_t    bsect;
    uint8_t     type;
    struct bpb;
    uint32_t    database;
    uint32_t    fatbase;
    uint32_t    n_fatent;
};

int get_fat(struct fatfs *fs, int clust);
int mount(struct fatfs *fs);
int walk_fat(struct fatfs *fs);

#endif

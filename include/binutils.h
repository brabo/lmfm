#ifndef __BIN_H
#define __BIN_H

int mount(struct fatfs *fs);
int ls(struct fatfs *fs, char *path);
int cat(struct fatfs *fs, char *path);
int touch(struct fatfs *fs, char *path);
int mk_file(struct fatfs *fs, char *path, uint8_t *buf, int len);
int edit_file(struct fatfs *fs, char *path);

/* VNZ NOTE: seems out of place in binutils, while having a vfs_ prefix? */
int vfs_ls(char *path);
int vfs_mount_fat(struct fatfs *fs);
void vfs_cat(char *path);

#endif

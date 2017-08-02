#ifndef __BIN_H
#define __BIN_H

int mount(struct fatfs *fs);
int ls(struct fatfs *fs, char *path);
int cat(struct fatfs *fs, char *path);
int touch(struct fatfs *fs, char *path);
int mk_file(struct fatfs *fs, char *path, uint8_t *buf, int len);

#endif

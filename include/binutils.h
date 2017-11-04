#ifndef __BIN_H
#define __BIN_H

int ls(char *path);
int mount(struct fatfs *fs);
void cat(char *path);
void fuzz(char *path);
int mk_file(char *path, uint8_t *buf, int len);
int edit_file(char *path);
int rm(char *path);

#endif

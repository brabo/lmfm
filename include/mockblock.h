#ifndef __MB_H
#define __MB_H

int mb_read(int fd, void *_buf, uint32_t lba, int offset, int count);
int mb_write(int fd, void *_buf, uint32_t lba, int offset, int count);
int mb_init(char *fname);
int mb_close(int fd);

#endif

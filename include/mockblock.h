#ifndef __MB_H
#define __MB_H

int mb_read(int fd, void *_buf, uint32_t lba, int offset, int count);
int mb_init(char *fname);

#endif

#ifndef __MB_H
#define __MB_H

int mb_read(struct fnode *fno, void *_buf, uint32_t lba, int offset, int count);
int mb_write(struct fnode *fno, void const *_buf, uint32_t lba, int offset, int count);
int mb_init(char *mb_name);
int mb_close(void);

#endif

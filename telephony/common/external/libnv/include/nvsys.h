#ifndef _LIBNVSYS_H_
#define _LIBNVSYS_H_
#ifdef __cplusplus
extern "C" {
#endif

/*
 * read_nv_item: read the nv item data
 * nv_item_id: ID of nv item which you read
 * readbuf: buffer where receive nv item data from nv system
 * return:
 *     return the real length of nv item
 *     -1 error
*/
long read_nv_item(unsigned long nv_item_id,unsigned char* readbuf);

/*
 * write_nv_item: write a nv item data
 * nv_item_id: ID of nv item which you want to write
 * writebuf: buffer where send nv item data to nv system
 * size: length of nv item data
 * return:
 *     0  success
 *     -1 error
*/
int write_nv_item(unsigned long nv_item_id,unsigned char* writebuf,unsigned long size);
#ifdef __cplusplus
}
#endif

#endif //_LIBNVSYS_H_

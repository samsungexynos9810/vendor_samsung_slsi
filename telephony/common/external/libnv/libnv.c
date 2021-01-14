#include <stdio.h>
#include "nv.h"
#include <nvdef.h>

/*
 * read_nv_item: read the nv item data
 * nv_item_id: ID of nv item which you read
 * readbuf: buffer where receive nv item data from nv system
 * return:
 *     return the real length of nv item
 *     -1 error
*/
long read_nv_item(unsigned long nv_item_id,unsigned char* readbuf)
{
    long ret = 0;
    unsigned long length = 0;
    nv_item_t nv_item_lo;
    int i;
    int nv_item_num = sizeof(nv_item_table)/sizeof(nv_item_table[0]);

    if(readbuf == NULL){
        printf("Invalid buffer point!\n");
        return -1;
    }

    for(i=0;i<nv_item_num;i++){
        nv_item_lo = nv_item_table[i];
        if((nv_item_lo.nv_id.nv_base+nv_item_lo.nv_id.nv_offset) == nv_item_id){
            break;
        }
    }
    if(i == nv_item_num){
        printf("Invalid NV ID: %ld\n",nv_item_id);
        return -1;
    }

    nv_item_init();
    ret = get_nv_item(readbuf,nv_item_id,&length);
    if(ret == -1){
        length = 0;
    }
    nv_item_finish();

    if(length != nv_item_lo.nv_length){
        printf("Invalid NV length: %ld\n",length);
        return -1;
    }

    return length;
}

/*
 * write_nv_item: write a nv item data
 * nv_item_id: ID of nv item which you want to write
 * writebuf: buffer where send nv item data to nv system
 * size: length of nv item data
 * return:
 *     0  success
 *     -1 error
*/
int write_nv_item(unsigned long nv_item_id,unsigned char* writebuf,unsigned long size)
{
    int ret;
    nv_item_t nv_item_lo;
    int i;
    int nv_item_num = sizeof(nv_item_table)/sizeof(nv_item_table[0]);

    if(writebuf == NULL){
        return -1;
    }

    for(i=0;i<nv_item_num;i++){
        nv_item_lo = nv_item_table[i];
        if((nv_item_lo.nv_id.nv_base+nv_item_lo.nv_id.nv_offset) == nv_item_id){
            if(nv_item_lo.nv_length == size){
                break;
            }
        }
    }
    if(i == nv_item_num){
        printf("Please check the NV ID: %ld\n",nv_item_id);
        return -1;
    }

    nv_item_init();

    ret = set_nv_item(writebuf,nv_item_id,size);
    nv_item_finish();

    return ret;
}

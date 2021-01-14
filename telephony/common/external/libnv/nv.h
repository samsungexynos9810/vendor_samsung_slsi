#ifndef __NV_H__
#define __NV_H__

typedef unsigned char uint8_t;

#define MAX_NV_ITEM_NUM            5000
#define NV_ITEM_FILE                "/nvp/nv_packages.bin"

struct nv_item {
    unsigned long    item_id;
    int         item_flag;
    unsigned long    item_size;
    unsigned char    *nv_item_data;
};

struct nv_item_head
{
    unsigned int        nv_item_aera_offset;    //the offset of nv_item structure
    unsigned int        total_nv_item_num;       //number of nv_item structure
    unsigned long    nv_data_area_offset;       //offset of item_data in nv_item structure
    unsigned long    nv_data_area_size;       //actual size of item_data in nv_item structure
    struct nv_item    *nv_items;
};

long set_nv_item(uint8_t* nv_data_buf,unsigned long nv_id,unsigned long size);

long get_nv_item(uint8_t* nv_data_buf,unsigned long nv_id,unsigned long* size);

int nv_item_init();
void nv_item_finish();

#endif //__NV_H__

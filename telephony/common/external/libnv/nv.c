#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "nv.h"

//#define NV_LOG_PRINT
#define LOG_PRINT() printf("Error occur at %s-->%d\n",__func__,__LINE__)

int fd_nv = 0;
pthread_mutex_t mutex;
struct nv_item_head nv_item_header;

void* zero_alloc(size_t size)
{
    void* mem = NULL;
    mem = malloc(size);
    memset(mem,0,size);
    return mem;
}

long get_nv_item(uint8_t* nv_data_buf,unsigned long nv_id,unsigned long* size)
{
    unsigned int i;
    int retval;

    //Judge if the conditions are valid.
    if(nv_data_buf == NULL){
        printf("No valid buffer to receive nv data!\n");
        return -1;
    }

    retval  = pthread_mutex_lock(&mutex);
    if(retval != 0) return retval;
    //get nv item data
    for(i=0;i<nv_item_header.total_nv_item_num;i++){
        if(nv_item_header.nv_items[i].item_id == nv_id){
            memcpy(nv_data_buf,nv_item_header.nv_items[i].nv_item_data,nv_item_header.nv_items[i].item_size);
            *size = nv_item_header.nv_items[i].item_size;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);

    return retval;
}

int modify_nv_item(uint8_t* nv_data_buf,unsigned long nv_id,unsigned long size,int index)
{
    int i;
    int retval;
    int nv_data_offset;
    unsigned long id = nv_id;
    unsigned long len = size;

    if(size != nv_item_header.nv_items[index].item_size){
        printf("The really size of %ld is %ld\n",nv_id,nv_item_header.nv_items[index].item_size);
        return -1;
    }
    memcpy(nv_item_header.nv_items[index].nv_item_data,nv_data_buf,nv_item_header.nv_items[index].item_size);

    nv_data_offset = nv_item_header.nv_data_area_offset;
    for(i=0;i<index;i++){
        nv_data_offset += nv_item_header.nv_items[i].item_size;
    }
    retval = lseek(fd_nv,nv_data_offset,SEEK_SET);
    if(retval < 0){
        LOG_PRINT();
        return -1;
    }
    retval = write(fd_nv,nv_item_header.nv_items[index].nv_item_data,nv_item_header.nv_items[index].item_size);
    if(retval < 0){
        LOG_PRINT();
        return -1;
    }

    return 0;
}

static int nv_cache_init();
int add_nv_item(uint8_t* nv_data_buf,unsigned long nv_id,unsigned long size)
{
    int retval;

    //add new nv item info.
    nv_item_header.nv_items = (struct nv_item*)realloc(nv_item_header.nv_items,
            ((nv_item_header.total_nv_item_num+1)*sizeof(struct nv_item)));
    if(nv_item_header.nv_items == NULL){
        LOG_PRINT();
        return -1;
    }
    nv_item_header.nv_items[nv_item_header.total_nv_item_num].item_id = nv_id;
    nv_item_header.nv_items[nv_item_header.total_nv_item_num].item_size = size;
    nv_item_header.nv_items[nv_item_header.total_nv_item_num].nv_item_data = (uint8_t*)zero_alloc(size);
    if(nv_item_header.nv_items[nv_item_header.total_nv_item_num].nv_item_data == NULL)
    {
        LOG_PRINT();
        goto free_nv_items;
    }
    memcpy(nv_item_header.nv_items[nv_item_header.total_nv_item_num].nv_item_data,nv_data_buf,size);

    /*sync to file.*/
    //write nv item
    retval = lseek(fd_nv,
        nv_item_header.nv_item_aera_offset+nv_item_header.total_nv_item_num*sizeof(struct nv_item),
        SEEK_SET);
    if(retval < 0){
        LOG_PRINT();
        goto free_nv_item_data;
    }
    retval = write(fd_nv,&nv_item_header.nv_items[nv_item_header.total_nv_item_num],sizeof(struct nv_item));
    if(retval < 0){
        LOG_PRINT();
        goto free_nv_item_data;
    }

    //write nv item data
    retval = lseek(fd_nv,nv_item_header.nv_data_area_offset+nv_item_header.nv_data_area_size,SEEK_SET);
    if(retval < 0){
        LOG_PRINT();
        goto free_nv_item_data;
    }
    retval = write(fd_nv,nv_item_header.nv_items[nv_item_header.total_nv_item_num].nv_item_data,size);
    if(retval < 0){
        LOG_PRINT();
        goto free_nv_item_data;
    }

    //write header
    nv_item_header.total_nv_item_num += 1;
    nv_item_header.nv_data_area_size += size;
    retval = lseek(fd_nv,0,SEEK_SET);
    if(retval < 0){
        LOG_PRINT();
        goto free_nv_item_data;
    }
    retval = write(fd_nv,&nv_item_header,sizeof(nv_item_header));
    if(retval < 0){
        LOG_PRINT();
        goto free_nv_item_data;
    }
    return 0;

free_nv_item_data:
    free(nv_item_header.nv_items[nv_item_header.total_nv_item_num].nv_item_data);
    //nv_cache_init();
free_nv_items:
    free(nv_item_header.nv_items);
    return -1;
}

long set_nv_item(uint8_t* nv_data_buf,unsigned long nv_id,unsigned long size)
{
    int retval;
    unsigned int i;
    int nv_flag,index;

    //Judge if the conditions are valid.
    if(nv_data_buf == NULL){
        printf("No valid buffer to receive nv data!\n");
        return -1;
    }

//Judge if the nv_id is a new nv item.
    nv_flag = 1;
    for(i=0;i<nv_item_header.total_nv_item_num;i++){
        if(nv_item_header.nv_items[i].item_id == nv_id){
            nv_flag = 0;
            index = i;
            break;
        }
    }

    retval  = pthread_mutex_lock(&mutex);
    if(retval != 0) return retval;
    //set nv item data
    if(nv_flag == 0){ //modify a existed nv item
        retval = modify_nv_item(nv_data_buf,nv_id,size,index);
    }else if(nv_flag == 1){ //add a new nv item
        retval = add_nv_item(nv_data_buf,nv_id,size);
    }

    pthread_mutex_unlock(&mutex);
    return retval;
}

static int nv_cache_init()
{
    int retval;
    int nread,nwrite;
    unsigned int i;
    unsigned int nfree = 0;

    /*Read the head structure from file.*/
    memset(&nv_item_header,0,sizeof(nv_item_header));
    retval = read(fd_nv,&nv_item_header,sizeof(nv_item_header));
    if((retval < 0) || (retval != sizeof(nv_item_header))){
        printf("read nv item header error:\nactually read: %d\n",retval);
        return -1;
    }
    if(nv_item_header.total_nv_item_num > MAX_NV_ITEM_NUM){
        printf("numbers of nv item is error!\nactual number: %d\n",nv_item_header.total_nv_item_num);
        memset(&nv_item_header,0,sizeof(nv_item_header));
        return -1;
    }
    if(nv_item_header.total_nv_item_num == 0){
        memset(&nv_item_header,0,sizeof(nv_item_header));
    }
    if(nv_item_header.nv_data_area_size == 0){
        printf("The header structure is error!\n");
        memset(&nv_item_header,0,sizeof(nv_item_header));
        return -1;
    }

    #ifdef NV_LOG_PRINT
    printf("nv_item_header.total_nv_item_num: %d\n",nv_item_header.total_nv_item_num);
    printf("nv_item_header.nv_item_aera_offset: %d\n",nv_item_header.nv_item_aera_offset);
    printf("nv_item_header.nv_data_area_offset: %ld\n",nv_item_header.nv_data_area_offset);
    printf("nv_item_header.nv_data_area_size: %ld\n",nv_item_header.nv_data_area_size);
    #endif

    /*Read the nv_item structure from file.*/
    //Assign memory to nv_items and get the nv_item structure from file to cache.
    nv_item_header.nv_items = zero_alloc(nv_item_header.total_nv_item_num * sizeof(struct nv_item));
    if(nv_item_header.nv_items == NULL){
        printf("Failed to assign memory for nv item!\n");
        return -1;
    }
    retval = lseek(fd_nv,nv_item_header.nv_item_aera_offset,SEEK_SET);
    if(retval < 0){
        LOG_PRINT();
        goto free_nv_items;
    }
    #ifdef NV_LOG_PRINT

    #endif
    retval = read(fd_nv,nv_item_header.nv_items,(nv_item_header.total_nv_item_num * sizeof(struct nv_item)));
    if(retval < 0){
        LOG_PRINT();
        goto free_nv_items;
    }

    //Assign memory to item_data and get the nv item data from file to cache.
    retval = lseek(fd_nv,nv_item_header.nv_data_area_offset,SEEK_SET);
    if(retval < 0){
        LOG_PRINT();
        goto free_nv_items;
    }
    unsigned long nv_data_offset = 0;
    for(i=0;i<nv_item_header.total_nv_item_num;i++){
        nv_item_header.nv_items[i].nv_item_data = zero_alloc(nv_item_header.nv_items[i].item_size);
        if(nv_item_header.nv_items[i].nv_item_data == NULL){
            LOG_PRINT();
            nfree = i;
            goto free_nv_item_data;
        }
        nread = read(fd_nv,nv_item_header.nv_items[i].nv_item_data,nv_item_header.nv_items[i].item_size);
        if(nread < 0){
            LOG_PRINT();
            nfree = i+1;
            goto free_nv_item_data;
        }
        nv_data_offset += nv_item_header.nv_items[i].item_size;
        retval = lseek(fd_nv,nv_item_header.nv_data_area_offset+nv_data_offset,SEEK_SET);
        if(retval < 0){
            LOG_PRINT();
            nfree = i+1;
            goto free_nv_item_data;
        }
    }

    return retval;

free_nv_item_data:
    for(i=0;i<nfree;i++){
        free(nv_item_header.nv_items[i].nv_item_data);
    }
free_nv_items:
    free(nv_item_header.nv_items);
    return -1;
}

int nv_item_init()
{
    int ret = 0;
    ret = pthread_mutex_init(&mutex, NULL);
    if(ret != 0) {
        return ret;
    }
    fd_nv = open(NV_ITEM_FILE,O_RDWR);
    if(fd_nv < 0){
        printf("open file failed!\n");
        return fd_nv;
    }
    #ifdef NV_LOG_PRINT
    printf("open_nv_file ret: %d\n",ret);
    #endif

    ret = nv_cache_init();
    #ifdef NV_LOG_PRINT
    printf("init_nv_cache ret: %d\n",ret);
    #endif
    if(ret < 0) {
        close(fd_nv);
        return ret;
    }

    return ret;
}

void nv_item_finish()
{
    unsigned int i;

    for(i=0;i<nv_item_header.total_nv_item_num;i++){
        free(nv_item_header.nv_items[i].nv_item_data);
    }
    free(nv_item_header.nv_items);

    if(fd_nv > 0){
        close(fd_nv);
        fd_nv = 0;
    }
}

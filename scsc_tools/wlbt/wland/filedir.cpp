#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <fcntl.h>

#define LOG_TAG "WLBTLOGDBG"
#include <utils/Log.h>

#include "filedir.h"

void makedir(const char* path)
{
    char fullpath[255] = {0,};
    char subdirpath[255] = {0,};
    char* subdir;
    int skipcount = 2 /*Skip "/dir1" and "/dir1/dir2" */;
    
    if(path[0] != '/')
    {
        fullpath[0] = '/';
        strncpy(fullpath+1, path, sizeof(fullpath)-2);
    }
    else
    {
        strncpy(fullpath, path, sizeof(fullpath)-1);
    }
    if(fullpath[strlen(fullpath)-1] != '/')
    {
        fullpath[strlen(fullpath)] = '/';
        fullpath[strlen(fullpath)+1] = 0;
    }
    subdir = fullpath+1;
    while((subdir = strstr(subdir, "/")))
    {
        if(skipcount > 0)
        {
            skipcount--;
            subdir = subdir+1;
            continue;
        }
        strncpy(subdirpath, fullpath, subdir - fullpath);
        mkdir(subdirpath, 0777);
        chmod(subdirpath, 0777);
        subdir = subdir+1;
    }
}

int create_dir(const char* file)
{
    char dir_path[255];
    char* output_dir_end = NULL;
    int str_len;

    memset(dir_path, 0, sizeof(dir_path));
    str_len = strlen(file);
    /* [Static Analysis] CID:249773 */
    strncpy(dir_path, file, sizeof(dir_path)-1);
    for(int i = str_len -1 ; i > -1 ; i--)
    {
        if(output_dir_end == NULL && dir_path[i] == '/')
        {
            output_dir_end = dir_path + i;
            break;
        }
    }
    if(output_dir_end == NULL)
    {
        return -1;
    }
    *output_dir_end = 0;
    makedir(dir_path);
    return 0;
}

FILE* create_file(const char* file)
{
    int fd = -1;
    FILE* fp = NULL;
    fd = open(file, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    if (fd == -1)
    {
        return NULL;
    }
    close(fd);
    chmod(file, 0666);
    fp = fopen(file, "w");
    return fp;
}

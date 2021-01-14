/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>

#include "vcd.h"

//#define DEBUG_HEXDUMP

int32_t commandcmp(const char *src, const char *dst, int length)
{
      char *source, *destination;
      int res, lcounterdst, lcountersrc;
      char *temp_src, *temp_dst;


      temp_src = (char *)malloc(length + 1);
      if ( NULL == temp_src ){
            perror("temp_src failed");
            return -1;
      }

      memset(temp_src, '\0', length + 1);
      temp_dst = (char *)malloc(length + 1);
      if ( NULL == temp_dst ){
            perror("temp_dst failed");
            free(temp_src);     //prevent
            return -1;
       }

       memset(temp_dst, '\0', length + 1);

       /*Convert to lower case*/
      for ( lcountersrc = 0; lcountersrc < length; lcountersrc++ ){
            temp_src[lcountersrc] = tolower(src[lcountersrc]);
      }
      for ( lcounterdst = 0; lcounterdst < length; lcounterdst++ ){
            temp_dst[lcounterdst] = tolower(dst[lcounterdst]);
      }

      for ( source = temp_src, destination = temp_dst; 0 < length; ++source, ++destination, length-- ){
            res = *source - *destination;
            if ( 0 != res )
                    break;
       }
       free(temp_src);
       temp_src = NULL;

       free(temp_dst);
       temp_dst = NULL;

       return res;
}

 int32_t set_sysfs(const char *devnm, const char *cmd)
{
    int fd;
    int r;

    ALOGD("%s(): set sysfs [%s] : %s\n", __FUNCTION__, devnm, cmd);

    fd = open(devnm, O_RDWR | O_NONBLOCK );
    if (fd == -1) {
        ALOGE("%s: open %s failed. (%d)", __FUNCTION__, devnm, errno);
        return -1;
    }

    r = write(fd, cmd, strlen(cmd));
    if (r == -1) {
        ALOGD("%s: write failed. (%d)", __FUNCTION__, errno);
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

 int32_t GetSysFS(const char *devnm, char *buffer)
{
    int fdGet;
    int r;

    fdGet = open(devnm, O_RDONLY | O_NONBLOCK);
    if (fdGet == -1) {
         ALOGE("%s: open %s failed. (%d)", __FUNCTION__, devnm, errno);
        return -1;
    }

    r = read(fdGet, buffer, 100);
    if (r == -1) {
        ALOGD("%s: read failed. (%d)", __FUNCTION__, errno);
        close(fdGet);
        return -1;
    }

    if (r > 0) {
        buffer[r] = '\0';
    }

    close(fdGet);
    return r;
}



static char buffer[2048] = {0, };
void PrintATCommands(const char *tag, const char *cmds, int n)
{
    static const char *CR = "<CR>";
    static const char *LF = "<LF>";

    if (cmds != NULL && n > 0) {
        unsigned int i = 0;
        unsigned int p = 0;
        memset(buffer, 0, sizeof(buffer));
        if (tag != NULL && *tag != 0) {
            snprintf(buffer, sizeof(buffer), "%s: ", tag);
        }

        for (i = 0, p = (unsigned int)strlen(buffer); i < (unsigned int)n; i++) {
            if (p + 4 > sizeof(buffer) - 1) {
                ALOGE("%s", buffer);
                *buffer = 0;
                p = 0;
            }

            if (cmds[i] == '\n') {
                strncat(buffer, LF, strlen(LF));
                p += (unsigned int)strlen(LF);
            }
            else if (cmds[i] == '\r') {
                strncat(buffer, CR, strlen(CR));
                p += (unsigned int)strlen(CR);
            }
            else {
                buffer[p++] = cmds[i];
                buffer[p] = 0;
            }
        } // end for i~

        if (*buffer != 0) {
            ALOGE("%s", buffer);
        }
    }
}

void HexDump(const char *data, size_t datalen)
{
    int i;
    int current = 0;
    int size = 0;
    char linebuf[1024] = {0, };

#ifdef DEBUG_HEXDUMP
    if (data != NULL && datalen > 0) {
        for (i = 0; i < datalen; i++) {
            if (i != 0 && i % 32 == 31) {
                ALOGE("%s", linebuf);
                *linebuf = 0;
            }
            else {
                sprintf(linebuf + strlen(linebuf), "%02X ", data[i]);
            }
        }
        if (*linebuf != 0) {
            ALOGE("%s", linebuf);
        }
    }
#endif // DEBUG_HEXDUMP
}

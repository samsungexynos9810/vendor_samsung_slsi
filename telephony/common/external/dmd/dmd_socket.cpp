/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "dmd_main.h"


int32_t g_profile_state = 0;

void run_socket_monitor(int32_t* modem_fd)
{
    int32_t serv_sock = -1;
    int32_t client_sock = -1;
    fd_set readfds;
    struct sockaddr_un client_addr;
    socklen_t addrlen;
    int32_t fd = *modem_fd;

    char socket_buf[SOCKET_MAX_BUF];
    int32_t nread = 0;

    std::string profilePath(gSlogPath);
    profilePath += "sbuff_profile.sdm";
    //Check Profile file exists --i.e. silent mode
    if( access( profilePath.c_str(), F_OK ) != -1 ){
        ALOGD("Profile File Exists");
        CheckModemStatus(fd);
        g_dmd_mode = MODE_SILENT;
        set_cp2usb_path(false);
        g_silent_logging_asked=1;
    }

    if( g_dmd_mode == MODE_OFF) {
        //start default mode
        ALOGD("Start default mode : ShannonDM mode");
        g_dmd_mode = MODE_NNEXT;
    }

    if ((serv_sock = socket_local_server(SOCKET_DMD, ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM)) < 0) {
        ALOGE("Fail to create socket, errno = %d", errno);
        return;
    }

    addrlen = sizeof(client_addr);

    while(1)
    {
        if ((client_sock = accept(serv_sock, (struct sockaddr*)&client_addr, &addrlen)) < 0 ) {
            ALOGE("%s : accept() fails, errno = %d", __func__, errno);
            close(serv_sock);
            return;
        }
        ALOGD("New client is connected...");

        while(1) {
            FD_ZERO(&readfds);
            FD_SET(client_sock, &readfds);

            if (select(client_sock+1, &readfds, NULL, NULL, NULL) < 0) {
                if (errno == EINTR) {
                    continue;
                }
                ALOGE("%s : Fail to monitor %s, error = %d", __func__, SOCKET_DMD, errno);
                break;
            }

            if (FD_ISSET(client_sock, &readfds)) {
                memset(socket_buf, 0x0, sizeof(socket_buf));
                nread = recv(client_sock, socket_buf, sizeof(socket_buf), 0);
                //ALOGD("nread =%d ", nread);

                if (nread < 0) {
                    if(errno == EINTR || errno == EAGAIN) {
                        continue;
                    }
                    ALOGE("%s : Fail to read %s, error = %d", __func__, SOCKET_DMD, errno);
                    break;
                } else if (nread == 0) {
                    ALOGE("%s : No data on %s", __func__, SOCKET_DMD);
                    break;
                }

                if (socket_buf[0] == 0x0B) { //silent
                    ALOGD("Set to Silent Logging mode");
                    if(g_dmd_mode == MODE_NNEXT) {
                        set_cp2usb_path(false);
                        gTimeForFile = CalcTime();
                        MergeHeader(gTimeForFile);     // insert header in first file for first time after boot
                    }
                    else if(g_dmd_mode == MODE_OBDM) {

                    }
                    g_dmd_mode = MODE_SILENT;
                    g_profile_state = PROFILE_SEND;
                    g_silent_logging_asked=1;
                    //property_set(PROPERTY_SILENTLOG_MODE, "On");
                    if (socket_buf[1] == 0x00){
                        g_profile_state = PROFILE_SEND_DONE;
                        ALOGD("PROFILE_SEND_DONE");
                        }
                    set_cp2usb_path(false);
                } else if ((nread > 2) && (g_profile_state == PROFILE_SEND)) { //profile
                    ALOGD("Send Profile data to CP");
                    if (EnableLogDump()){
                        log_hexdump((const char *)socket_buf, nread);
                    }
                    DoSaveProfile(socket_buf,nread);
                    dmd_modem_write(fd, socket_buf, nread); //write to modem
                } else if (socket_buf[0] == 0x0E) { //nnext
                    ALOGD("Set to ShannonDM mode");
                    if(g_dmd_mode == MODE_SILENT) {
                        g_silent_logging_started = 0;
                        g_silent_logging_asked = 0;
                        if (remove(profilePath.c_str()) != 0)    // remove profile
                        {
                            ALOGE("%s : remove error(%s)", __FUNCTION__, strerror(errno));
                        }
                        OnSilentLogStop();    // reset buffers and silent log variables --ResetSilentLogValues
                    }
                    else if(g_dmd_mode == MODE_OBDM) {

                    }
                    property_set(PROPERTY_SILENTLOG_MODE, "");
                    g_dmd_mode = MODE_NNEXT;
                } else if (socket_buf[0] == 0x09) { //unknown
                } else if (socket_buf[0] == 0x0A) { //save
                    ALOGD("Save CP DM message into files");
                    trigger_save_dump();
                    if ((send(client_sock, socket_buf , 1, MSG_EOR)) < 0) {
                        ALOGD("send failed for trigger_save_dump = %d\n  %s",serv_sock, strerror(errno));
                    } else {
                        ALOGD("----------Sent trigger_save_dump complete----------\n");
                    }
                } else if (socket_buf[0] == 0x7f) {
                    ALOGD("Request to some DM command. for example stop req sending DM message");
                    if (EnableLogDump()){
                        log_hexdump((const char *)socket_buf, nread);
                    }
                    //write to modem, stop req, start/stop tcp packet req. etc
                    dmd_modem_write(fd, socket_buf, nread);
                }
            }
        }
    }
    return;
}

int CheckModemStatus(int32_t fd)
{
    //check modem status start
    int status = 0;
    int spin = 100;

    while (spin--)
    {
        status = ioctl(fd, IOCTL_MODEM_STATUS);
        if (status == STATE_ONLINE)
        {
            ALOGD("%s : Modem  is ONLINE", __func__);
            gTimeForFile = CalcTime();
            MergeHeader(gTimeForFile);     // insert header in first file for first time after boot
            DoSendProfile(fd);
            return 1;
        }
        usleep(500000);
    }

    if (spin < 0)
    {
        ALOGE("%s : Modem boot timeout", __func__);
    }
    return 0;
}

void DoSaveProfile(char * buff, uint32_t count)
{

    FILE *sprofile_file=NULL;

    char filestr[100];
    sprintf(filestr, "%ssbuff_profile.sdm", gSlogPath);

    sprofile_file = fopen(filestr, "ab");

    if(NULL!=sprofile_file)
    {
        fwrite(buff, sizeof(buff[0]), count, sprofile_file);
        fclose(sprofile_file);
        sprofile_file=NULL;
    }
    else
    {
        ALOGD("----------Opening sprofile_file Fail-----\n");
    }
}

void DoSendProfile(int32_t fd)
{
    FILE *sprofile_file = NULL;
    char filestr[100];
    int read_num = 0;
    struct stat ProfileFileSt;
    static char *read_buf = NULL;

    if (read_buf == NULL)
    {
        read_buf = (char *)malloc(SOCKET_MAX_BUF);
        memset(read_buf, 0, SOCKET_MAX_BUF);

        if (read_buf == NULL)
        {
            ALOGE("%s : memory allocation fail", __FUNCTION__);
            return;
        }
    }

    sprintf(filestr, "%ssbuff_profile.sdm", gSlogPath);
    ALOGD("%s----------File String is  : %s  ----------\n", __func__, filestr);

    sprofile_file = fopen(filestr, "rb");

    if (NULL != sprofile_file)
    {
        if (stat(filestr, &ProfileFileSt) != 0)
        {
            ALOGE("%s : stat error(%s)", __FUNCTION__, strerror(errno));
        }
        if (0 < (read_num = fread(read_buf, sizeof(read_buf[0]), ProfileFileSt.st_size, sprofile_file)))
        {
            log_hexdump((const char *)read_buf, read_num);
            dmd_modem_write(fd, read_buf, read_num); //write to modem
        }
        else
            ALOGE("%s : fread error(%s)", __FUNCTION__, strerror(errno));

        fclose(sprofile_file);
        sprofile_file = NULL;
    }
    else
    {
        ALOGD("----------Opening sprofile_file Fail-----\n");
    }
}

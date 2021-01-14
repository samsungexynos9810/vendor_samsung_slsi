/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/prctl.h>
#include <sys/capability.h>
#include <sys/system_properties.h>

#define LOG_TAG "WLBTLOGDBG"
#include <utils/Log.h>

#include <string>
#include <sstream>
#include "cutils/sockets.h"
#include <private/android_filesystem_config.h>

#define SOCKET_WLAND "socket_wland"
#define SOCKET_WLAND_BUFFER_SIZE 200

#include "common.h"
#include "RouteMonitor.h"
#include "SingleShotTimer.h"
#include "wlbtlog.h"
#include "livemxlog.h"
#include "picojson.h"

#define UNUSED(p)   ((void)(p))

#define BUILD_TYPE_PROPERTY "ro.build.type"
#define DEFAULT_BUILD_TYPE "user"
static char buildtype[PROP_VALUE_MAX + 1];

#define LIVE_MXLOG_PROPERTY "exynos.wlbtlog.livemxlog"
static char mxlogforward[PROP_VALUE_MAX + 1];
static char mxlogforwardprev[PROP_VALUE_MAX + 1];

static dujeonglee::basiclibrary::SingleShotTimer<1, 1> readKlogTimer;
static dujeonglee::basiclibrary::SingleShotTimer<1, 1> readRoutingTableTimer;
static dujeonglee::basiclibrary::SingleShotTimer<1, 1> readMxlogForwarding;

void catch_sigchld(int sig)
{
    int status;
    pid_t pid;
    UNUSED(sig);

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        ALOGD("CHILD EXIT %d", pid);
    }
}

int32_t main(void)
{
    int32_t serverSockFd = -1;
    int32_t clientSockFd = -1;
    struct sockaddr_un clientAddr;
    socklen_t len;
    char buf[SOCKET_WLAND_BUFFER_SIZE];
    int recvcount = 0;
    fd_set readfds;

    memset(buildtype, 0, sizeof(buildtype));
    memset(mxlogforward, 0, sizeof(mxlogforward));
    memset(mxlogforwardprev, 0, sizeof(mxlogforwardprev));
    /**
    * Read build type from ro.build.type
    */
    if(__system_property_get(BUILD_TYPE_PROPERTY, buildtype) == 0)
    {
        strcpy(buildtype, DEFAULT_BUILD_TYPE);
    }
    ALOGD("%s = %s", BUILD_TYPE_PROPERTY, buildtype);

    /**
    * Start monitoring exynos.wlbtlog.livemxlog
    */
    readMxlogForwarding.PeriodicTask(1000, []() -> bool {
        strcpy(mxlogforwardprev, mxlogforward);
        if(__system_property_get(LIVE_MXLOG_PROPERTY, mxlogforward) == 0)
        {
            strcpy(mxlogforward, "none");
        }
        /**
        * When livemxlog is changed.
        */
        if(strcmp(mxlogforward, mxlogforwardprev))
        {
            if(strcmp(mxlogforward, "none") == 0)
            {
                ALOGD("Stop MXLOG");
                LiveMxLog::stop();
            }
            else if(strcmp(mxlogforward, "wifi_normal") == 0)
            {
                ALOGD("Start WifiNormal");
                LiveMxLog::start_wifi_normal();
            }
            else if(strcmp(mxlogforward, "bt_normal") == 0)
            {
                ALOGD("Start BTNormal");
                LiveMxLog::start_bt_normal();
            }
            else if(strcmp(mxlogforward, "bt_audio") == 0)
            {
                ALOGD("Start BTAudio");
                LiveMxLog::start_bt_audio();
            }
            else
            {
                ALOGD("Unsupported mode %s :Stop MXLOG", mxlogforward);
                LiveMxLog::stop();
            }
        }
        return true;
    });

    /**
    * Start monitor kernel routing changes
    */
    readRoutingTableTimer.PeriodicTask(0, []() -> bool {
        RouteMonitor::Instance()->MonitorRoutingUpdate();
        return true;
    });

    if ((serverSockFd = socket_local_server(SOCKET_WLAND, ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM)) < 0)
    {
        ALOGE("Fail to create socket, errno = %d", errno);
        return -1;
    }

    struct sigaction sact;
    sact.sa_flags = 0 | SA_NODEFER; //Queing for SIGCHLD
    sigemptyset(&sact.sa_mask);

    //signal handler registration
    sact.sa_handler = catch_sigchld;
    sigaction(SIGCHLD, &sact, NULL);

    Common::LoadConfiguration();

    while (1)
    {
        ALOGD("Server is waiting client...");
        len = sizeof(struct sockaddr_un);
        if ((clientSockFd = accept(serverSockFd, (struct sockaddr *)&clientAddr, &len)) < 0)
        {
            continue;
        }
        ALOGD("New client is connected...");
        while (1)
        {
            FD_ZERO(&readfds);
            FD_SET(clientSockFd, &readfds);

            if (select(clientSockFd + 1, &readfds, NULL, NULL, NULL) < 0)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                break;
            }

            if (FD_ISSET(clientSockFd, &readfds))
            {
                memset(buf, 0x0, sizeof(buf));
                int data_length = 0;
                int remainingbytes = 0;
                picojson::value json_obj;
                /* TCP stream has no boundary for data.
                 * Data can be split into multiple recvs.
                 */
                recvcount = recv(clientSockFd, &data_length, sizeof(data_length), 0);
                if (recvcount < 0)
                {
                    if (errno == EINTR || errno == EAGAIN)
                    {
                        continue;
                    }
                    ALOGE("%s : Fail to monitor wland client %d, errno = %d",
                        __func__, clientSockFd, errno);
                    break;
                }
                else if (recvcount == 0)
                {
                    ALOGE("%s : No data on %s", __func__, SOCKET_WLAND);
                    break;
                }
                else if(recvcount != sizeof(recvcount))
                {
                    ALOGE("%s : Incomplete data on %s", __func__, SOCKET_WLAND);
                    break;
                }
                data_length = ntohl(data_length);
                ALOGD("data_length: %d [%d]", data_length, recvcount);
                remainingbytes = data_length;
                while(remainingbytes > 0)
                {
                    recvcount = recv(clientSockFd, buf + (data_length - remainingbytes), remainingbytes, 0);
                    if (recvcount < 0)
                    {
                        if (errno == EINTR || errno == EAGAIN)
                        {
                            continue;
                        }
                        ALOGE("%s : Fail to monitor wland client %d, errno = %d",
                            __func__, clientSockFd, errno);
                        break;
                    }
                    else if (recvcount == 0)
                    {
                        ALOGE("%s : No data on %s", __func__, SOCKET_WLAND);
                        break;
                    }
                    remainingbytes = remainingbytes - recvcount;
                }
                if(remainingbytes > 0)
                {
                    ALOGE("Connection lost");
                    break;
                }
                ALOGD("JSON: %s", buf);
                const std::string json_string(buf);
                std::istringstream json_stream(json_string);
                try
                {
                    json_stream >> json_obj;
                }
                catch(const std::exception &ex)
                {
                    ALOGD("JSON parsing error");
                    continue;
                }
                if(json_obj.is<picojson::object>())
                {
                    try{
                        if (strncmp(json_obj.get("name").get<std::string>().c_str(), "wifilog", 7) == 0)
                        {
                            if(strncmp(json_obj.get("exec").get<std::string>().c_str(), "start", 5) == 0)
                            {
                                if(strncmp(json_obj.get("option").get<std::string>().c_str(), "mxlog", 5) == 0)
                                {
                                    WlbtLog::start_mxlog(json_obj.get("dir").get<std::string>().c_str(), clientSockFd);
                                }
                                else if(strncmp(json_obj.get("option").get<std::string>().c_str(), "udilog", 6) == 0)
                                {
                                    WlbtLog::start_udilog(json_obj.get("dir").get<std::string>().c_str(), clientSockFd);
                                }
                                else if(strncmp(json_obj.get("option").get<std::string>().c_str(), "all", 3) == 0)
                                {
                                    const std::string mxlogname = json_obj.get("dir").get<std::string>() + "_mxlog";
                                    const std::string udilogname = json_obj.get("dir").get<std::string>() + "_udilog";

                                    WlbtLog::start_mxlog(mxlogname.c_str(), clientSockFd);
                                    WlbtLog::start_udilog(udilogname.c_str(), clientSockFd);
                                }
                                else
                                {
                                    const std::string mxlogname = json_obj.get("dir").get<std::string>() + "_mxlog";
                                    const std::string udilogname = json_obj.get("dir").get<std::string>() + "_udilog";

                                    WlbtLog::start_mxlog(mxlogname.c_str(), clientSockFd);
                                    WlbtLog::start_udilog(udilogname.c_str(), clientSockFd);
                                }
                            }
                            else if(strncmp(json_obj.get("exec").get<std::string>().c_str(), "stop", 4) == 0)
                            {
                                if(strncmp(json_obj.get("option").get<std::string>().c_str(), "mxlog", 5) == 0)
                                {
                                    WlbtLog::stop_mxlog();
                                }
                                else if(strncmp(json_obj.get("option").get<std::string>().c_str(), "udilog", 6) == 0)
                                {
                                    WlbtLog::stop_udilog();
                                }
                                else if(strncmp(json_obj.get("option").get<std::string>().c_str(), "all", 3) == 0)
                                {
                                    WlbtLog::stop_mxlog();
                                    WlbtLog::stop_udilog();
                                }
                                else
                                {
                                    WlbtLog::stop_mxlog();
                                    WlbtLog::stop_udilog();
                                }
                            }
                        }
                        else if (strncmp(json_obj.get("name").get<std::string>().c_str(), "btlog", 5) == 0)
                        {
                            if(strncmp(json_obj.get("exec").get<std::string>().c_str(), "start", 5) == 0)
                            {
                                const std::string mxlogname = json_obj.get("dir").get<std::string>() + "_mxlog";
                                const std::string udilogname = json_obj.get("dir").get<std::string>() + "_udilog";

                                if(strncmp(json_obj.get("option").get<std::string>().c_str(), "general", 7) == 0)
                                {
                                    ALOGD("btlog start general");
                                    WlbtLog::start_bt_normal_log(mxlogname.c_str(), clientSockFd);
                                    WlbtLog::start_udilog(udilogname.c_str(), clientSockFd);
                                }
                                else if(strncmp(json_obj.get("option").get<std::string>().c_str(), "audio", 5) == 0)
                                {
                                    ALOGD("btlog start audio");
                                    WlbtLog::start_bt_audio_log(mxlogname.c_str(), clientSockFd);
                                    WlbtLog::start_udilog(udilogname.c_str(), clientSockFd);
                                }
                                else if(strncmp(json_obj.get("option").get<std::string>().c_str(), "custom", 6) == 0)
                                {
                                    ALOGD("btlog start custom");
                                    WlbtLog::start_bt_custom_log(mxlogname.c_str(), clientSockFd, json_obj.get("data").get<std::string>().c_str());
                                    WlbtLog::start_udilog(udilogname.c_str(), clientSockFd);
                                }
                            }
                            else if(strncmp(json_obj.get("exec").get<std::string>().c_str(), "stop", 4) == 0)
                            {
                                WlbtLog::stop_bt_log();
                                WlbtLog::stop_udilog();
                            }
                        }
                    }
                    catch (const std::exception &ex)
                    {
                        ALOGD("JSON parsing exception. Stop all logging daemon.");
                        WlbtLog::stop_mxlog();
                        WlbtLog::stop_bt_log();
                        WlbtLog::stop_udilog();
                    }
                }
                else
                {
                    system(buf);
                }
            }
        }
    }
    return 0;
}

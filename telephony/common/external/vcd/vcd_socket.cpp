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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "cutils/sockets.h"

#include "vcd.h"
#include "vcd_socket.h"

#define PATH_SOCKET "/dev/socket/app_stream"
#define SOCKET_VCD "/dev/socket/vcd_stream"

#define SOCKET_MAX_BUF 4096
int32_t client_sock = -1;
int32_t g_current_type;

extern const char *g_ims_startWith[13];

//#define AT_STRING       "AT+CGMI\r\n"  //remember to put CR and LF after each AT command

int32_t write_to_interface(int32_t fd, char* buffer, int32_t buf_len);
int get_modem_fd(void);
int get_modem_write_from_app(void);
int set_modem_write_from_app(int appswitch);

void *run_socket_monitor()
{
    int32_t serv_sock = -1;

    fd_set readfds;
    struct sockaddr_un serv_soc_addr;
    struct sockaddr_un client_addr;
    socklen_t addrlen;
    int32_t status;

    char socket_buf[SOCKET_MAX_BUF];
    int32_t nread = 0;

     if ((serv_sock = socket(AF_UNIX, SOCK_STREAM,0)) < 0) {
        ALOGE("Fail to create socket, errno = %d", errno);
        return NULL;
    }

     memset(&serv_soc_addr, 0x0, sizeof(struct sockaddr_un));

     serv_soc_addr.sun_family = AF_UNIX;
    strncpy(serv_soc_addr.sun_path, PATH_SOCKET, sizeof(serv_soc_addr.sun_path) -1);
    unlink(serv_soc_addr.sun_path);

    if (bind(serv_sock, (struct sockaddr*)&serv_soc_addr, sizeof(struct sockaddr_un)) < 0 ) {
        ALOGE("Fail to bind socket, error = %d", errno);
        close(serv_sock);
        return NULL;
    }

    if (listen(serv_sock, 1) < 0) {
        ALOGE("Fail to listen, error = %d", errno);
        close(serv_sock);
        return NULL;
    }

    addrlen = sizeof(client_addr);

    while(1)
    {
        if ((client_sock = accept(serv_sock, (struct sockaddr*)&client_addr, &addrlen)) < 0 ) {
            ALOGE("%s : accept() fails, errno = %d", __func__, errno);
            close(serv_sock);
            return NULL;
        }
        ALOGD("New client is connected...");

        while(1) {
            FD_ZERO(&readfds);
            FD_SET(client_sock, &readfds);

            if (select(client_sock+1, &readfds, NULL, NULL, NULL) < 0) {
                if (errno == EINTR) {
                    continue;
                }
                ALOGE("%s : Fail to monitor %s, error = %d", __func__, PATH_SOCKET, errno);
                break;
            }

            if (FD_ISSET(client_sock, &readfds)) {
                memset(socket_buf, 0x0, sizeof(socket_buf));
                nread = recv(client_sock, socket_buf, sizeof(socket_buf), 0);

                if (nread < 0) {
                    if(errno == EINTR || errno == EAGAIN) {
                        continue;
                    }
                    ALOGE("%s : Socket Closed %s", __func__, PATH_SOCKET);
                    break;
                } else if (nread == 0) {
                    ALOGE("%s : No data on %s", __func__, PATH_SOCKET);
                    break;
                }

                ALOGD("%s : Read %s socket_buf = %s, nread=%d", __func__, PATH_SOCKET, socket_buf, nread);
                //ALOGD("socket_buf[0]=%02x", socket_buf[0]);

                if(socket_buf[0] == 0x0A && nread==1)
                {
                    //turn switch on
                    set_modem_write_from_app(1);
                    ALOGD("%s :set_modem_write_from_app TRUE", __func__);
                }else if (socket_buf[0] == 0x0B && nread==1){
                    //turn switch off
                    set_modem_write_from_app(0);
                    ALOGD("%s :set_modem_write_from_app FALSE", __func__);
                }else{
                    //write to modem
                    if(get_modem_write_from_app()){
                        write_to_interface(get_modem_fd(), socket_buf, nread);
                    }
                }

            }
        }
    }
    return NULL;
}

int get_client_socket(void)
{
return client_sock;
}




////////////////////////////////////////////////////////////////////////////////
// APIs
////////////////////////////////////////////////////////////////////////////////

#define INVALID_FD (-1)
#define MAX_CLIENT_CONN 5
#define MAX_BUF_SIZE    2048

pthread_t g_tid;
static CommandResponseHandler g_fpResponseHandler = NULL;
static CommandRequestHandler g_fpRequestHandler = NULL;
static void *ConnectivityServerThreadProc(void *arg);

int g_ServerSock = INVALID_FD;
struct str_ClientSock {
    int fd;
    int type;
    int id;
};
struct str_ClientSock g_ClientSock[MAX_CLIENT_CONN] = { {INVALID_FD, 0, 0}, };
int g_ClientSize = 0;

static int InitServerSock()
{
    g_ServerSock = socket_local_server(SOCKET_VCD, ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
    if (g_ServerSock < 0) {
        ALOGE("Error: socket_local_server fail. sockname=%s", SOCKET_VCD);
        g_ServerSock = INVALID_FD;
        return -1;
    }
    ALOGE("Initialize server socket: serversock=%d", g_ServerSock);
    return 0;
}

static int CloseServerSock()
{
    if (g_ServerSock != INVALID_FD) {
        ALOGE("Close server socket");
        close(g_ServerSock);
        g_ServerSock = INVALID_FD;
    }
    return 0;
}

static int ResetServerSock()
{
    ALOGE("ResetServerSock");
    int ret = -1;
    CloseServerSock();
    ret = InitServerSock();
    return ret;
}

static int GetMaxFd()
{
    int i = 0;
    int max = g_ServerSock;
    for (i = 0; i < g_ClientSize; i++) {
        if (g_ClientSock[i].fd != INVALID_FD) {
            if (max < g_ClientSock[i].fd) {
                max = g_ClientSock[i].fd;
            }
        }
    } // end for i ~
    return max;
}

static int UpdateFdSet(fd_set *fdset)
{
    int i;
    if (fdset == NULL) {
        ALOGE("Error: Invalid fdset");
        return -1;
    }

    FD_ZERO(fdset);
    if (g_ServerSock != INVALID_FD) {
        FD_SET(g_ServerSock, fdset);
    }

    ALOGE("current client size=%d", g_ClientSize);
    for (i = 0; i < g_ClientSize; i++) {
        if (g_ClientSock[i].fd != INVALID_FD) {
            FD_SET(g_ClientSock[i].fd, fdset);
        }
    } // end for i ~
    return 0;

}

static int AddClient(int fd)
{
    int i;
    if (fd > 0) {
        for (i = 0; i < MAX_CLIENT_CONN; i++) {
            if (g_ClientSock[i].fd == INVALID_FD) {
                g_ClientSock[i].fd = fd;
                g_ClientSize++;
                ALOGE("Add new client: fd=%d", fd);
                return 0;
            }
        } // end for i ~
        ALOGE("Not allow new connection: g_ClientSize=%d", g_ClientSize);
        return -1;
    }

    // not available connection
    ALOGE("Error: Invalid fd=%d", fd);
    return -1;
}

static void RemoveClient(int index)
{
    if (g_ClientSock[index].fd > 0) {
        g_current_type = TYPE_NORMAL;
        g_ClientSock[index].fd = INVALID_FD;
        g_ClientSock[index].type = INVALID_FD;
        g_ClientSock[index].id = INVALID_FD;
        g_ClientSize--;
        ALOGE("Remove client: fd=%d", g_ClientSock[index].fd);
        return ;
    }
    ALOGE("Error: Invalid fd=%d", g_ClientSock[index].fd);
}

/**
*InitConnectivityServer
*/
int StartConnectivityServer(CommandResponseHandler responseCallback, CommandRequestHandler requestCallback)
{
    int i;
    ALOGE("%s", __FUNCTION__);

    for (i = 0; i < MAX_CLIENT_CONN; i++) {
        g_ClientSock[i].fd = INVALID_FD;
        g_ClientSock[i].type = INVALID_FD;
        g_ClientSock[i].id = INVALID_FD;
    } // end for i ~

    if (pthread_create(&g_tid, NULL, ConnectivityServerThreadProc, NULL) != 0) {
        ALOGE("%s: Error: pthread_create fail", __FUNCTION__);
        return -1;
    }
    ALOGE("%s: Register callback g_fpResponseHandler=0x%p", __FUNCTION__, responseCallback);
    ALOGE("%s: Register callback g_fpRequestHandler=0x%p", __FUNCTION__, requestCallback);

    g_fpResponseHandler = responseCallback;
    g_fpRequestHandler = requestCallback;
    return 0;
}

/**
*CloseConnectivityServer
*/
void StopConnectivityServer()
{
    // TODO TBD
}

/**
*GetConnectivityClient
*/
int SendDataToConnectivity(const char *data, unsigned int datalen, int id)
{
    int count = 0;
    int i;

    if (data == NULL || *data == 0) {
        ALOGE("%s: Invalid parameter", __FUNCTION__);
        return -1;
    }

    if (datalen == 0 || datalen >= MAX_BUF_SIZE) {
        ALOGE("%s: Invalid data size", __FUNCTION__);
        return -1;
    }

    if (g_ServerSock == INVALID_FD) {
        ALOGE("%s: server sock is not initialized.", __FUNCTION__);
        return -1;
    }

    for (i = 0; i < g_ClientSize; i++) {
        if (g_ClientSock[i].fd > 0 && g_ClientSock[i].id == id) {
            if (write(g_ClientSock[i].fd, data, datalen) > 0) {
                ALOGE("%s: send data to fd=%d", __FUNCTION__, g_ClientSock[i].fd);
                count++;
                //write(g_ClientSock[i].fd, "\0", 1);
            }
            else {
                // print error
            }
        }
    } // end for i ~
    ALOGD("%s: id = %d, ", __FUNCTION__, id);
    return count;
}

static void AcceptConnection(int fd)
{
    int conn = -1;
    if (fd == INVALID_FD) {
        ALOGE("%s: Invalid fd", __FUNCTION__);
        return ;
    }

    conn = accept(fd, NULL, NULL);
    if (conn == -1) {
         ALOGE("%s: failed to accept client", __FUNCTION__);
         close(conn);
    } else {
        ALOGE("%s: New client connection is accepted: fd=%d", __FUNCTION__, conn);
        if (AddClient(conn) < 0) {
            // not allow new client connection
            ALOGE("%s: Add client fail", __FUNCTION__);
            close(conn);
        }
    }
}
void setClientSockType(int index, char *data)
{
    ALOGE("%s: index = %d", __FUNCTION__, index);
    const char *str_nv = "\r\n+VTYPE:1,nv";
    const char *str_ims = "\r\n+VTYPE:1,ims";
    const char *str_app = "\r\n+VTYPE:2,app";

    if(strncasecmp(data, str_nv, strlen(str_nv)) == 0) {
        g_ClientSock[index].type = TYPE_NORMAL;
        g_ClientSock[index].id = ID_NV;
    } else if (strncasecmp(data, str_ims, strlen(str_ims)) == 0) {
        ALOGD("VCD: Support AIMS command***********");
        g_ClientSock[index].type = TYPE_NORMAL;
        g_ClientSock[index].id = ID_IMS;
    } else if (strncasecmp(data, str_app, strlen(str_app)) == 0) {
        ALOGD("VCD: Support APP command***********");
        g_current_type = TYPE_APP;
        g_ClientSock[index].type = TYPE_APP;
        g_ClientSock[index].id = ID_APP;
    } else {

    }
}
static void ProcessSocketData(int index)
{
    int nread = 0;
    char buf[MAX_BUF_SIZE+1] = {0, };
    static const char *startWith = "\r\n+VTYPE:";
    if (g_ClientSock[index].fd == INVALID_FD) {
        ALOGE("%s: Invalid fd", __FUNCTION__);
        return ;
    }
    nread = read(g_ClientSock[index].fd, buf, MAX_BUF_SIZE);
    if (nread <= 0) {
        ALOGE("%s: read fail or disconnected. fd=%d", __FUNCTION__, g_ClientSock[index].fd);
        RemoveClient(index);
        close(g_ClientSock[index].fd);
    } else if (strncasecmp(buf, startWith, strlen(startWith)) == 0) {
        ALOGD("buf : %s", buf);
        setClientSockType(index, buf);
    } else {
        ALOGE("%s: read %d byte(s)", __FUNCTION__, nread);
        buf[nread] = 0;
        ALOGD("buf : %s", buf);
        // callback response handler
        if (g_fpResponseHandler != NULL && g_fpRequestHandler != NULL) {
            if (g_ClientSock[index].id == ID_NV) {
                g_fpResponseHandler(DEVICE_TYPE_CONNECTIVITY, buf, nread);
            } else if (g_ClientSock[index].id == ID_IMS) {
                g_fpResponseHandler(DEVICE_TYPE_IMSCONNECTIVITY, buf, nread);
            } else if (g_ClientSock[index].id == ID_APP) {
                g_fpRequestHandler(DEVICE_TYPE_APP, buf, nread);
            }
        }
    }
}

// thread handler
void *ConnectivityServerThreadProc(void *arg)
{
    fd_set fdset;
    int maxfd = 0;
    int err = 0;

    ALOGE("%s: Thread is Running", __FUNCTION__);
    while (1) {
        if (g_ServerSock == INVALID_FD) {
            if (InitServerSock() < 0) {
                sleep(1);
                continue;
            }
        }

        maxfd = GetMaxFd();
        UpdateFdSet(&fdset);

        err = select(maxfd + 1, &fdset, NULL, NULL, NULL);
        if (err > 0) {
            if (FD_ISSET(g_ServerSock, &fdset)) {
                AcceptConnection(g_ServerSock);
            }
            else {
                int i;
                for (i = 0; i < g_ClientSize; i++) {
                    if (FD_ISSET(g_ClientSock[i].fd, &fdset)) {
                        ALOGD("ProcessSocketData -> index = %d", i);
                        ProcessSocketData(i);
                    }
                } // end for i ~
            }
        }
        else if (errno == EINTR) {
            ALOGE("thread got signal");
            break;
        }
    } // end while ~
    ALOGE("%s: Thread is terminated", __FUNCTION__);
    return NULL;
}

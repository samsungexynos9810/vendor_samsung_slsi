#include <string.h>
#include <unistd.h>

#define LOG_TAG "WLBTLOGDBG"
#include <utils/Log.h>

#include "common.h"
#include "filedir.h"
#include "udilog.h"

bool volatile UdiLog::running = false;
int          UdiLog::pid = -1;
FILE*        UdiLog::readfd = NULL;
FILE*        UdiLog::writefd = NULL;
int          UdiLog::logfilesize = 0;
unsigned int UdiLog::logid = 0;
char         UdiLog::prefix[256];
std::function<void()>                           UdiLog::removefilter = NULL;
dujeonglee::basiclibrary::SingleShotTimer<2, 1> UdiLog::worker;

void UdiLog::clear_resource()
{
    char cmd[128];

    ALOGD("Clear resource UDI");
    if(pid > 0)
    {
        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "kill -9 %d", pid);
        system(cmd);
        pid = -1;
    }
    if(removefilter)
    {
        try
        {
            removefilter();
        }
        catch(const std::exception& ex)
        {
            ALOGD("Cannot call removefilter");
        }
    }
    if(readfd != NULL)
    {
        fclose(readfd);
        readfd = NULL;
    }
    if(writefd != NULL)
    {
        fclose(writefd);
        writefd = NULL;
    }
    logid = 0;
    logfilesize = 0;
    memset(prefix, 0, sizeof(prefix));
    running = false;
}

int UdiLog::start(std::function<void()> addfilter, std::function<void()> rmfilter, const char* prefixstr, std::function<void(char*)> callback)
{
    pid_t children_pid = -1;
    int pipefd_1[2];
    int pipefd_2[2];

    if(running)
    {
        return -1;
    }
    Common::update_max_file_size();
    Common::update_max_files();
    running = true;
    memset(prefix, 0, sizeof(prefix));
    if(prefixstr)
    {
        /* [Static Analysis] CID:249797 */
        strncpy(prefix, prefixstr, sizeof(prefix)-1);
    }
    logfilesize = 0;
    logid = 0;

    if(addfilter)
    {
        try
        {
            addfilter();
        }
        catch(const std::exception& ex)
        {
            ALOGD("Cannot call addfilter");
        }
    }
    if(rmfilter)
    {
        removefilter = rmfilter;
    }

    if(pipe(pipefd_1) == -1)
    {
        return -1;
    }
    pid = fork();
    if(pid==0)
    {
        dup2(pipefd_1[1], STDOUT_FILENO);
        close(pipefd_1[0]);
        close(pipefd_1[1]);
        execl("/vendor/bin/slsi_wlan_udi_log", "/vendor/bin/slsi_wlan_udi_log", "stdout", (char*) NULL);
    }
    else
    {
        if(pipe(pipefd_2) == -1)
        {
            close(pipefd_1[1]);
            close(pipefd_1[0]);
            return -1;
        }
        children_pid=fork();

        if(children_pid==0)
        {
            dup2(pipefd_1[0], STDIN_FILENO);
            dup2(pipefd_2[1], STDOUT_FILENO);
            close(pipefd_1[1]);
            close(pipefd_1[0]);
            close(pipefd_2[1]);
            close(pipefd_2[0]);
            execl("/vendor/bin/slsi_wlan_udi_log_decode", "/vendor/bin/slsi_wlan_udi_log_decode", "stdin", (char*) NULL);
        }
        else
        {
            close(pipefd_1[0]);
            close(pipefd_1[1]);
            close(pipefd_2[1]);
            readfd = fdopen(pipefd_2[0], "r");
            if(readfd == NULL)
            {
                return -1;
            }
            ALOGD("Start UDI");
            worker.PeriodicTask(0, [callback]() -> bool {
                char        buffer[2048] = {0};
                char        file[256] = {0};
                char*       ret;
                fd_set      readfds;

                if(!running)
                {
                    return false;
                }
                if(logfilesize == 0 && prefix[0] != 0)
                {
                    if(writefd != NULL)
                    {
                        fclose(writefd);
                        writefd = NULL;
                    }
                    sprintf(file, "%s_%u.log", prefix, logid);
                    if(create_dir(file) < 0)
                    {
                        clear_resource();
                        return false;
                    }
                    if((writefd = create_file(file)) == NULL)
                    {
                        clear_resource();
                        return false;
                    }
                    logfilesize++;
                    sprintf(file, "%s_%u.log", prefix, (logid++ - Common::max_files));
                    remove(file);
                }
                timeval to = {
                    .tv_sec = 0,
                    .tv_usec = 500000
                };
                const int fd = fileno(readfd);
                FD_ZERO(&readfds);
                FD_SET(fd, &readfds);
                const int state = select(fd + 1, &readfds, (fd_set *)0, (fd_set *)0, &to);
                switch(state)
                {
                    case -1:
                        return true;
                    case 0:
                        return true;
                }
                if((ret = fgets(buffer, sizeof(buffer), readfd)) == NULL)
                {
                    clear_resource();
                    return false;
                }
                if(prefix[0] != 0)
                {
                    if(fputs(buffer, writefd) == -1)
                    {
                        clear_resource();
                        return false;
                    }
                    logfilesize += (unsigned int)strlen(buffer);
                    if(logfilesize > Common::max_file_size * (1024 * 1024))
                    {
                        logfilesize = 0;
                    }
                }
                else
                {
                    callback(buffer);
                }
                return true;
            }, 1);
        }
    }
    return pid;
}

void UdiLog::stop()
{
    worker.ImmediateTaskNoExcept([](){
        clear_resource();
    }, 0);
    while(running);
    ALOGD("Stop UDI");
}

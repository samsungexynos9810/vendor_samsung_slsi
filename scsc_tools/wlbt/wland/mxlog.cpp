#include <string.h>
#include <unistd.h>

#define LOG_TAG "WLBTLOGDBG"
#include <utils/Log.h>

#include "common.h"
#include "filedir.h"
#include "mxlog.h"

bool volatile MxLog::running = false;
int          MxLog::pid = -1;
FILE*        MxLog::readfd = NULL;
FILE*        MxLog::writefd = NULL;
int          MxLog::logfilesize = 0;
unsigned int MxLog::logid = 0;
char         MxLog::prefix[256];
std::function<void()>                           MxLog::removefilter = NULL;
dujeonglee::basiclibrary::SingleShotTimer<2, 1> MxLog::worker;

void MxLog::clear_resource()
{
    char cmd[128];

    ALOGD("Clear resource MX");
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
        catch(const std::exception &ex)
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

int MxLog::start(std::function<void()> addfilter, std::function<void()> rmfilter, const char* prefixstr, std::function<void(char*)> callback)
{
    int pipefd[2];

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
        /* [Static Analysis] CID:249786 */
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
        catch(const std::exception &ex)
        {
            ALOGD("Cannot call addfilter");
        }
    }
    if(rmfilter)
    {
        removefilter = rmfilter;
    }

    if(pipe(pipefd) == -1)
    {
        return -1;
    }
    pid = fork();
    if(pid==0)
    {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execl("/vendor/bin/cat", "/vendor/bin/cat", "/sys/kernel/debug/scsc/ring0/samsg", (char*) NULL);
    }
    else
    {
        close(pipefd[1]);
        readfd = fdopen(pipefd[0], "r");
        if(readfd == NULL)
        {
            return -1;
        }
        ALOGD("Start MX");
        worker.PeriodicTask(0, [callback]() -> bool {
            char        buffer[2048] = {0};
            char        file[256] = {0};
            char*       ret;
            fd_set      readfds;
            time_t      now = time(0);
            struct tm   tstruct;
            size_t      timestamp_length;
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
                ALOGD("file %s", file);
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

            /* Write timestamp in the head */
            tstruct = *localtime(&now);
            timestamp_length = strftime(buffer, sizeof(buffer), "%Y-%m-%d.%X", &tstruct);
            buffer[timestamp_length] = ' ';
            if((ret = fgets(buffer + timestamp_length + 1, sizeof(buffer) - timestamp_length - 1, readfd)) == NULL)
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
    return pid;
}

void MxLog::stop()
{
    worker.ImmediateTaskNoExcept([](){
        clear_resource();
    }, 0);
    while(running);
    ALOGD("Stop MX");
}

#ifndef SLSI_WLBTLOG_H
#define SLSI_WLBTLOG_H

class WlbtLog
{
private:
    static pid_t bt_logging_pid;
    static pid_t mxlog_pid;
    static pid_t udilog_pid;

public:
    static bool start_mxlog(const char* cmd, int res_sock);
    static bool start_udilog(const char* cmd, int res_sock);
    static bool start_bt_normal_log(const char* cmd, int res_sock);
    static bool start_bt_audio_log(const char* cmd, int res_sock);
    static bool start_bt_custom_log(const char* cmd, int res_sock, const char* data);

    static void setup_bt_normal_log_filter();
    static void setup_bt_audio_log_filter();
    static void setup_bt_custom_log_filter(const char* filter);
    static void revert_bt_log_filter();

    static void stop_mxlog();
    static void stop_udilog();
    static void stop_bt_log();
};

#endif
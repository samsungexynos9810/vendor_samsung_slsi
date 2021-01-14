#ifndef SLSI_COMMON_H
#define SLSI_COMMON_H

enum MxLogStatus
{
    MXLOG_OFF_T,
    MXLOG_WIFI_NORMAL_T,
    MXLOG_BT_NORMAL_T,
    MXLOG_BT_AUDIO_T
};

enum UdiLogStatus
{
    UDILOG_OFF_T,
    UDILOG_ON_T
};

#define DEFAULT_MAX_FILE_SIZE    (100) // 100MB
#define DEFAULT_MAX_FILES        (5)  // 5

class Common
{
private:
    static MxLogStatus  live_mxlog_status;
    static MxLogStatus  file_mxlog_status;
    static UdiLogStatus udilog_status;
    static void initDefaultValue();
    static void setPropertyValue(const char *buffer, int value);
    static int getPropertyValue(const char *buffer);
public:
    static void ChangeLiveMxLogState(MxLogStatus s);
    static void ChangeFileMxLogState(MxLogStatus s);
    static void ChangeUdiLogState(UdiLogStatus s);
    static void LoadConfiguration();
public:
    static int max_file_size;
    static int max_files;
    static void update_max_file_size();
    static void update_max_files();
    static void send_reply(int sock, int pid);
    static void get_time_string(char *buffer, unsigned int size);
};
#endif

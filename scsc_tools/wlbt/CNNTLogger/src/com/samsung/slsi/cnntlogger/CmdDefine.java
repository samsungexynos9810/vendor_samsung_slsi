package com.samsung.slsi.cnntlogger;

final class CmdDefine {

    static final String LOGTAG = "CNNTLog";
    static final String PACKAGE_NAME = "com.samsung.slsi.cnntlogger";
    static final String SOCKET_NAME = "socket_wland";
    static final String TYPE_AP = "logcat";
    static final String TYPE_MX = "mxlog";
    static final String TYPE_UDI = "udilog";

    static final String SYSTEM_PROPERTY_LOGGING_PATH = "exynos.wlbtlog.path";
    static final String SYSTEM_PROPERTY_SDCARD_PATH = "exynos.sdcard.path.log";
    static final String SDCARD_DEFAULT_DIR = "/sdcard/bbklog/";
    static final String SABLE_LOG_DIR = "/data/vendor/log/wifi/";

    static final String SDCARD_DIR = "/storage/sdcard/log/CNNTLog";
    static final String dirPath = "/data/vendor/log/wlbt/";
    static final String copyDir = "/CNNTLog";

    static final String KEY_BTN_STATUS = "Pref_LogButton";
    static final String KEY_CHECK_LOGCAT = "Pref_LogcatCheck";
    static final String KEY_CHECK_MXLOG = "Pref_MxlogCheck";
    static final String KEY_CHECK_UDILOG = "Pref_UdilogCheck";
    static final String KEY_AP_TYPE = "Pref_ApLoggingType";

    static final String KEY_IS_WIFI_LOG = "Pref_WifiLog";
    static final String KEY_BT_FILTER = "Pref_BtFilter";
    static final String KEY_BT_USE_DEFAULT_MODE = "Pref_DefaultMode";

    static final boolean FEATURE_USE_NOTIBAR = true;

    static final String StartScript = "SS";
    static final String StopScript = "SE";
    static final String DeleteLog = "DL";

    //udi_log
    static final String udiLog = "slsi_wlan_udi_log stdout | slsi_wlan_udi_log_decode stdin > ";

    //logcat
    static final String lcLog = "logcat -v time";

    //mxlog
    static final String mxLog = "cat /sys/kernel/debug/scsc/ring0/samsg | mxdecoder > ";

    //tcpdump
    static final String tcpdump = "tcpdump -i ";
    static final String tcpOption = " -p -s 0 -B 32768 -n -w ";

    //kernel
    static final String dmsgClr = "dmesg -c > /dev/null";
    static final String kmsg = "cat /proc/kmsg > ";

    //mibs
    static final String mibs = "slsi_wlan_mib --vif 1 2200";

    // Logging command
    static final String loggingStart = "start";
    static final String loggingStop = "stop";

    static final String wifiCommand = "wifilog";
    static final String btCommand = "btlog";
    static final String btNormalFilter = "normal";
    static final String btAudioFilter = "audio";
    static final String btCustomFilter = "custom";

    static final int WIFI_LOG_UDILOG_START = 0;
    static final int WIFI_LOG_MXLOG_START = 1;
    static final int WIFI_LOG_ALL_START = 2;
    static final int WIFI_LOG_UDILOG_STOP = 3;
    static final int WIFI_LOG_MXLOG_STOP = 4;
    static final int WIFI_LOG_ALL_STOP = 5;
    static final int BT_LOG_NORMAL = 6;
    static final int BT_LOG_AUDIO = 7;
    static final int BT_LOG_CUSTOM = 8;
    static final int BT_LOG_STOP = 9;

    static final String COPY_LOG_INTENT = "com.samsung.slsi.log.action.COPY_LOGGING";
    static final String CLEAR_LOG_INTENT = "com.samsung.slsi.log.action.CLEAR_LOGGING";
    static final String COPY_LOG_RESULT_INTENT = "com.samsung.slsi.log.action.COPY_RESULT";
    static final String CLEAR_LOG_RESULT_INTENT = "com.samsung.slsi.log.action.CLEAR_RESULT";

    static final String UPDATE_LOGGING_OPTION = "com.samsung.slsi.update.loggingOption";
    static final String WIFI_LOGGING_START_INTENT = "com.samsung.slsi.wifi.action.START_LOGGING";
    static final String WIFI_LOGGING_STOP_INTENT = "com.samsung.slsi.wifi.action.STOP_LOGGING";
    static final String BT_LOGGING_START_INTENT = "com.samsung.slsi.bt.action.START_LOGGING";
    static final String BT_LOGGING_STOP_INTENT = "com.samsung.slsi.bt.action.STOP_LOGGING";

    // VSC OPCode
    static final int VSC_OPCODE_DBFW = 0xFDF1;
    static final int VSC_OPCODE_LINK_LAYER = 0xFDF2;

    // VSC Subcode
    static final int DBFW_SCO_DUMP = 0x30;

    // VSC Parameters for SCO Dump
    static final int SCO_DUMP_DISABLE = 0x00;
    static final int SCO_PCM_TX_DUMP = 0x01;
    static final int SCO_PCM_RX_DUMP = 0x02;
    static final int SCO_PCM_TX_RX_DUMP = 0x03;
    static final int SCO_ANT_TX_DUMP = 0x04;
    static final int SCO_ANT_RX_DUMP = 0x05;
    static final int SCO_ANT_TX_RX_DUMP = 0x06;
    static final int SCO_ANT_PCM_TX_RX_DUMP = 0x07;

    // Those value should be in sync with enum MxLogStatus in common.h
    static final int MXLOG_STATUS_OFF = 0;
    static final int MXLOG_STATUS_WIFI_NORMAL = 1;
    static final int MXLOG_STATUS_BT_NORMAL = 2;
    static final int MXLOG_STATUS_BT_AUDIO = 3;
}

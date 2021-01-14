package com.samsung.slsi.cnntlogger;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class LoggingManager {

    private final String TAG = "LoggingManager";

    private Context mContext;

    private NotificationManager mNotificationManager;

    private Notification.Builder mBuilder;

    private NotificationChannel mChannel;

    private Notification mNotification;

    LoggingManager(Context context, boolean isNeedPopupActivity) {
        mContext = context;
        if (CmdDefine.FEATURE_USE_NOTIBAR) {
            initNotification(isNeedPopupActivity);
        }
    }

    private void initNotification(boolean isNeedPopupActivity) {
        Log.d(TAG, "initNotification " + isNeedPopupActivity);

        // Get Notification Service
        mNotificationManager = (NotificationManager) mContext.getSystemService(Context.NOTIFICATION_SERVICE);
        mChannel = new NotificationChannel(CmdDefine.PACKAGE_NAME, CmdDefine.LOGTAG, NotificationManager.IMPORTANCE_LOW);
        mChannel.enableLights(true);
        mNotificationManager.createNotificationChannel(mChannel);

        mBuilder = new Notification.Builder(mContext);
        mBuilder.setChannelId(CmdDefine.PACKAGE_NAME);
        mBuilder.setSmallIcon(R.drawable.rocket);
        mBuilder.setTicker(mContext.getString(R.string.noti_ticker));

        CNNTUtils utils = new CNNTUtils(mContext);
        if (utils.getPreference(CmdDefine.KEY_BTN_STATUS, true)) {
            mBuilder.setContentTitle(mContext.getString(R.string.noti_log_deactive));
            mBuilder.setContentText("");
            mChannel.setImportance(NotificationManager.IMPORTANCE_LOW);

        } else {
            mBuilder.setContentTitle(mContext.getString(R.string.noti_log_active));
            String logtype = mContext.getString(R.string.noti_log_type);
            if (utils.getPreference(CmdDefine.KEY_CHECK_LOGCAT, true)) logtype += " logcat";

            if (utils.getPreference(CmdDefine.KEY_IS_WIFI_LOG, true)) {
                if (utils.getPreference(CmdDefine.KEY_CHECK_MXLOG, true)) logtype += " mxlog";
                if (utils.getPreference(CmdDefine.KEY_CHECK_UDILOG, true)) logtype += " udilog";
            } else {
                String filter = utils.getPreference(CmdDefine.KEY_BT_FILTER, CmdDefine.btNormalFilter);
                logtype += filter;
            }

            mBuilder.setContentText(logtype);
            mChannel.setImportance(NotificationManager.IMPORTANCE_HIGH);
        }

        if (isNeedPopupActivity) {
            mBuilder.setContentIntent(PendingIntent.getActivity(mContext, 0,
                    new Intent(mContext, CNNTActivity.class), PendingIntent.FLAG_UPDATE_CURRENT));
        }
        mBuilder.setAutoCancel(!isNeedPopupActivity);
        mBuilder.setOngoing(isNeedPopupActivity);

        if (isNeedPopupActivity) {
            mNotification = mBuilder.build();
            mNotificationManager.notify(1234, mNotification);
        }
    }

    public void updateLoggingNotification(boolean isActive, String logType) {
        if (!CmdDefine.FEATURE_USE_NOTIBAR) {
            return;
        }
        mBuilder.setContentTitle(isActive ? mContext.getString(R.string.noti_log_active)
                : mContext.getString(R.string.noti_log_deactive));
        mBuilder.setContentText(isActive ? logType
                : mContext.getString(R.string.noti_log_break));
        mChannel.setImportance(isActive ? NotificationManager.IMPORTANCE_HIGH
                : NotificationManager.IMPORTANCE_LOW);
        mNotification = mBuilder.build();
        mNotificationManager.notify(1234, mNotification);
    }

    public void removeInstance() {
        if (mNotificationManager != null) {
            mNotificationManager.cancelAll();
            mNotificationManager = null;
        }
    }
}

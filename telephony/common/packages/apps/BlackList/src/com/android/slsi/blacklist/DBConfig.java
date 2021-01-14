/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.android.slsi.blacklist;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.net.Uri;
import android.provider.BaseColumns;


public class DBConfig {

    public static final String AUTHORITY = "com.android.slsi.blacklist.BlackListProvider";
    public static final String CONTENT_TYPE = "vnd.android.cursor.dir/vnd.blacklist.db";
    public static final String CONTENT_ITEM_TYPE = "vnd.android.cursor.item/vnd.blacklist.db";

    public static final int BLACKLISTINFO = 1;
    public static final int BLACKLISTINFO_ITEM = 2;
    public static final int CALLLOGINFO = 3;
    public static final int CALLLOGINFO_ITEM = 4;
    public static final int SMSINFO = 5;
    public static final int SMSINFO_ITEM = 6;

    //provider Constants related to blacklistinfo_table
    public static final class BlackListInfo implements BaseColumns {
        public static final Uri CONTENT_URI = Uri.parse("content://"
                + AUTHORITY + "/blacklistinfo");

        public static final String DEFAULT_SORT_ORDER = "number asc";
        public static final String TABLE_NAME = "blacklistinfo";
        public static final String NAME = "name";
        public static final String NUMBER = "number";
        public static final String INTERCEPT_TYPE = "type";

        //Intercept SMS and Call at the same time
        public static final int BLOCK_TYPE_ALL = 0;
        //Only intercept Call
        public static final int BLOCK_TYPE_CALL = 1;
        //Only intercept SMS
        public static final int BLOCK_TYPE_SMS = 2;

    }

    //provider Constants related to callloginfo_table
    public static final class CallLogInfo implements BaseColumns {
        public static final Uri CONTENT_URI = Uri.parse("content://"
                + AUTHORITY + "/callloginfo");

        public static final String DEFAULT_SORT_ORDER = "date desc";
        public static final String TABLE_NAME = "callloginfo";
        public static final String NUMBER = "number";
        public static final String DATE = "date";
        public static final String LOCATION = "location";
    }

    //provider Constants related to smsinfo_table
    public static final class SmsInfo implements BaseColumns {
        public static final Uri CONTENT_URI = Uri.parse("content://"
                + AUTHORITY + "/smsinfo");

        public static final String DEFAULT_SORT_ORDER = "date desc";
        public static final String NUMBER_DATE_SORT_ORDER = "number asc,date desc";
        public static final String TABLE_NAME = "smsinfo";

        public static final String NUMBER = "number";
        public static final String DATE = "date"; // ms
        public static final String DATE_SENT = "date_sent"; //
        public static final String PROTOCOL = "protocol";
        public static final String READ = "read"; // 0: unread 1:read
        public static final String REPLY_PATH_PRESENT = "reply_path_present";
        public static final String SUBJECT = "subject";
        public static final String BODY = "body";
        public static final String SERVICE_CENTER = "service_center";
        public static final String SMSTYPE = "smstype"; // SystemSms.TYPE
    }
    //provider Constants related to system Sms
    public static final class SystemSms {
        public static final String TYPE = "type";
        public static final int MESSAGE_TYPE_ALL = 0;
        public static final int MESSAGE_TYPE_INBOX = 1;
        public static final int MESSAGE_TYPE_SENT = 2;
        public static final int MESSAGE_TYPE_DRAFT = 3;
        public static final int MESSAGE_TYPE_OUTBOX = 4;
        public static final int MESSAGE_TYPE_FAILED = 5;
        public static final int MESSAGE_TYPE_QUEUED = 6;
    }
}

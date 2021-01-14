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

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.provider.BaseColumns;


public class BlackListDBHelper extends SQLiteOpenHelper {

    private static String DATABASE_NAME = "BlackList.db";
    private static int DATABASE_VERSION = 1;

    public BlackListDBHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE " + DBConfig.BlackListInfo.TABLE_NAME
                + "("
                + BaseColumns._ID + " INTEGER PRIMARY KEY,"
                + DBConfig.BlackListInfo.NUMBER + " TEXT,"
                + DBConfig.BlackListInfo.NAME + " TEXT,"
                + DBConfig.BlackListInfo.INTERCEPT_TYPE + " INTEGER"
                + ");");

        db.execSQL("CREATE TABLE " + DBConfig.CallLogInfo.TABLE_NAME
                + "("
                + BaseColumns._ID + " INTEGER PRIMARY KEY,"
                + DBConfig.CallLogInfo.NUMBER + " TEXT,"
                + DBConfig.CallLogInfo.DATE + " INTEGER,"
                + DBConfig.CallLogInfo.LOCATION + " TEXT"
                + ");");
        db.execSQL("CREATE TABLE " + DBConfig.SmsInfo.TABLE_NAME
                + "("
                + BaseColumns._ID + " INTEGER PRIMARY KEY,"
                + DBConfig.SmsInfo.NUMBER + " TEXT,"
                + DBConfig.SmsInfo.DATE + " INTEGER,"
                + DBConfig.SmsInfo.DATE_SENT + " INTEGER DEFAULT 0,"
                + DBConfig.SmsInfo.PROTOCOL + " INTEGER,"
                + DBConfig.SmsInfo.READ + " INTEGER DEFAULT 0,"
                + DBConfig.SmsInfo.REPLY_PATH_PRESENT + " INTEGER,"
                + DBConfig.SmsInfo.SUBJECT + " TEXT,"
                + DBConfig.SmsInfo.BODY + " TEXT,"
                + DBConfig.SmsInfo.SERVICE_CENTER + " TEXT,"
                + DBConfig.SmsInfo.SMSTYPE + " INTEGER"
                + ");");
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        db.execSQL("DROP TABLE IF EXISTS " + DBConfig.BlackListInfo.TABLE_NAME);
        db.execSQL("DROP TABLE IF EXISTS " + DBConfig.CallLogInfo.TABLE_NAME);
        db.execSQL("DROP TABLE IF EXISTS " + DBConfig.SmsInfo.TABLE_NAME);
        onCreate(db);

    }

}

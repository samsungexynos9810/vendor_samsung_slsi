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
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.CursorLoader;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;

public class BlackList{
    public long id = -1;
    public String phoneNo = null;
    public String name = null;
    public String date = null;
    public int intercept_type = -1;

    private static final int INVALID_ID = -1;

    private static final String[] PROJECTION = new String[]{
        DBConfig.BlackListInfo._ID,
        DBConfig.BlackListInfo.NAME,
        DBConfig.BlackListInfo.NUMBER,
        DBConfig.BlackListInfo.INTERCEPT_TYPE
    };

    public BlackList(){

    }
    BlackList(Cursor cursor){
        id = cursor.getLong(cursor.getColumnIndex(DBConfig.BlackListInfo._ID));
        name = cursor.getString(cursor.getColumnIndex(DBConfig.BlackListInfo.NAME));
        phoneNo = cursor.getString(cursor.getColumnIndex(DBConfig.BlackListInfo.NUMBER));
        intercept_type = cursor.getInt(cursor.getColumnIndex(DBConfig.BlackListInfo.INTERCEPT_TYPE));
    }

    public static ContentValues createConentValue(BlackList blackList){
        ContentValues values = new ContentValues(3);
        values.put(DBConfig.BlackListInfo.NAME, blackList.name);
        values.put(DBConfig.BlackListInfo.NUMBER, blackList.phoneNo);
        values.put(DBConfig.BlackListInfo.INTERCEPT_TYPE, blackList.intercept_type);

        return values;
    }

    public static CursorLoader getBlackListLoader(Context context){
        return new CursorLoader(context,DBConfig.BlackListInfo.CONTENT_URI, PROJECTION, null, null, null);
    }

    public static Uri getUri(long blackListId){
        return ContentUris.withAppendedId(DBConfig.BlackListInfo.CONTENT_URI, blackListId);
    }

    public static long getId(Uri uri){
        return ContentUris.parseId(uri);
    }
};

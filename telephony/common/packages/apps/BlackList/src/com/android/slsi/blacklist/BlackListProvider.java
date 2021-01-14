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

import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.text.TextUtils;
import android.util.Log;

public class BlackListProvider extends ContentProvider {

    private BlackListDBHelper mHelper;
    private static final UriMatcher mUriMatcher = new UriMatcher(
            UriMatcher.NO_MATCH);;

    static {
        mUriMatcher.addURI(DBConfig.AUTHORITY, "blacklistinfo", DBConfig.BLACKLISTINFO);
        mUriMatcher.addURI(DBConfig.AUTHORITY, "blacklistinfo/#", DBConfig.BLACKLISTINFO_ITEM);
        mUriMatcher.addURI(DBConfig.AUTHORITY, "callloginfo", DBConfig.CALLLOGINFO);
        mUriMatcher.addURI(DBConfig.AUTHORITY, "callloginfo/#", DBConfig.CALLLOGINFO_ITEM);
        mUriMatcher.addURI(DBConfig.AUTHORITY, "smsinfo", DBConfig.SMSINFO);
        mUriMatcher.addURI(DBConfig.AUTHORITY, "smsinfo/#", DBConfig.SMSINFO_ITEM);
    }

    @Override
    public String getType(Uri uri) {
        switch (mUriMatcher.match(uri)) {
        case DBConfig.BLACKLISTINFO:
        case DBConfig.CALLLOGINFO:
        case DBConfig.SMSINFO:
            return DBConfig.CONTENT_TYPE;
        case DBConfig.BLACKLISTINFO_ITEM:
        case DBConfig.CALLLOGINFO_ITEM:
        case DBConfig.SMSINFO_ITEM:
            return DBConfig.CONTENT_ITEM_TYPE;
        default:
            throw new IllegalArgumentException("Unknown URI " + uri);
        }
    }

    @Override
    public boolean onCreate() {
        mHelper = new BlackListDBHelper(getContext());
        return true;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        ContentValues contentValues;
        if (null != values) {
            contentValues = new ContentValues(values);
        } else {
            contentValues = new ContentValues();
        }
        SqlArguments args = new SqlArguments(uri);
        SQLiteDatabase db = mHelper.getWritableDatabase();
        switch (mUriMatcher.match(uri)) {
        case DBConfig.BLACKLISTINFO:
            if (contentValues.containsKey(DBConfig.BlackListInfo.NUMBER) == false) {
                throw new IllegalArgumentException("Invalid record");
            }
            if (contentValues.containsKey(DBConfig.BlackListInfo.INTERCEPT_TYPE) == false) {
                throw new IllegalArgumentException("Invalid record");
            }
            if (contentValues.containsKey(DBConfig.BlackListInfo.NAME) == false) {
                contentValues.putNull(DBConfig.BlackListInfo.NAME);
            }
            break;
        case DBConfig.CALLLOGINFO:
            if (contentValues.containsKey(DBConfig.CallLogInfo.NUMBER) == false) {
                throw new IllegalArgumentException("Invalid record");
            }
            if (contentValues.containsKey(DBConfig.CallLogInfo.DATE) == false) {
                contentValues.put(DBConfig.CallLogInfo.DATE, 0);
            }
            if (contentValues.containsKey(DBConfig.CallLogInfo.LOCATION) == false) {
                contentValues.putNull(DBConfig.CallLogInfo.LOCATION);
            }
            break;
        case DBConfig.SMSINFO:
            if (contentValues.containsKey(DBConfig.SmsInfo.NUMBER) == false) {
                contentValues.put(DBConfig.SmsInfo.NUMBER, "");
            }
            if (contentValues.containsKey(DBConfig.SmsInfo.DATE) == false) {
                contentValues.put(DBConfig.SmsInfo.DATE, 0);
            }
            if (contentValues.containsKey(DBConfig.SmsInfo.DATE_SENT) == false) {
                contentValues.put(DBConfig.SmsInfo.DATE_SENT, 0);
            }
            if (contentValues.containsKey(DBConfig.SmsInfo.PROTOCOL) == false) {
                contentValues.put(DBConfig.SmsInfo.PROTOCOL, 0);
            }
            if (contentValues.containsKey(DBConfig.SmsInfo.READ) == false) {
                contentValues.put(DBConfig.SmsInfo.READ, 0);
            }
            if (contentValues
                    .containsKey(DBConfig.SmsInfo.REPLY_PATH_PRESENT) == false) {
                contentValues.put(DBConfig.SmsInfo.REPLY_PATH_PRESENT, 0);
            }
            if (contentValues.containsKey(DBConfig.SmsInfo.SUBJECT) == false) {
                contentValues.put(DBConfig.SmsInfo.SUBJECT, "");
            }
            if (contentValues.containsKey(DBConfig.SmsInfo.BODY) == false) {
                contentValues.put(DBConfig.SmsInfo.BODY, "");
            }
            if (contentValues.containsKey(DBConfig.SmsInfo.SERVICE_CENTER) == false) {
                contentValues.put(DBConfig.SmsInfo.SERVICE_CENTER, "");
            }
            if (contentValues.containsKey(DBConfig.SmsInfo.SMSTYPE) == false) {
                contentValues.put(DBConfig.SmsInfo.SMSTYPE,
                        DBConfig.SystemSms.MESSAGE_TYPE_INBOX);
            }

            break;

        default:
            // invalidate the requested uri
            throw new IllegalArgumentException("Unknown URI " + uri);
        }
        long id = db.insertOrThrow(args.table, null, values);
        if (id > 0) {
            Uri newUri = ContentUris.withAppendedId(uri, id);
            getContext().getContentResolver().notifyChange(newUri, null);
            return newUri;
        }
        throw new SQLException("Failed to insert row into " + uri);
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        SqlArguments args = new SqlArguments(uri, selection, selectionArgs);
        SQLiteDatabase db = mHelper.getWritableDatabase();
        int count = db.delete(args.table, args.where, args.args);
        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs,
            String sortOrder) {
        SqlArguments args = new SqlArguments(uri, selection, selectionArgs);
        SQLiteDatabase db = mHelper.getWritableDatabase();
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        qb.setTables(args.table);
        Cursor c = qb.query(db, projection, args.where, args.args, null, null, sortOrder);
        c.setNotificationUri(getContext().getContentResolver(), uri);
        return c;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        SQLiteDatabase db = mHelper.getWritableDatabase();
        SqlArguments args = new SqlArguments(uri, selection, selectionArgs);
        int count = db.update(args.table, values, args.where, args.args);
        return count;
    }

    //Structure and obtain some parameters of the SQL, such as the name of the table, where and ags.
    //And judge whether the URI is effective, if it is invalid, then throw an exception.
    static class SqlArguments {
        public final String table;
        public final String where;
        public final String[] args;

        SqlArguments(Uri url, String where, String[] args) {
            this.table = url.getPathSegments().get(0);
            if (url.getPathSegments().size() == 1) {
                this.where = where;
                this.args = args;
            } else if (url.getPathSegments().size() != 2) {
                throw new IllegalArgumentException("Invalid URI: " + url);
            } else if (!TextUtils.isEmpty(where)) {
                throw new UnsupportedOperationException("WHERE clause not supported: " + url);
            } else {
                this.where = "_id=" + ContentUris.parseId(url) + (!TextUtils.isEmpty(where) ? " AND (" + where
                        + ")" : "");
                this.args = args;
            }
        }

        SqlArguments(Uri url) {
            if (url.getPathSegments().size() == 1) {
                table = url.getPathSegments().get(0);
                where = null;
                args = null;
            } else {
                throw new IllegalArgumentException("Invalid URI: " + url);
            }
        }
    }
}

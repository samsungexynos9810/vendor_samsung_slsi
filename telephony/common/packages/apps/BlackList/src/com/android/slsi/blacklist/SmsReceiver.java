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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Intent;
import android.database.Cursor;
import android.util.Log;
import android.net.Uri;
import java.util.Calendar;
import java.util.GregorianCalendar;
import android.telephony.SmsMessage;
import android.text.TextUtils;



public class SmsReceiver extends BroadcastReceiver {

    private final static String LOG_TAG = "SmsReceiver";
    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(LOG_TAG, "onReceive()...Intent: " + intent);
        String action = intent.getAction();
        if (action.equals("android.provider.Telephony.SMS_RECEIVED")) {
            SmsMessage[] sms = getMessagesFromIntent(intent);
            if (sms != null && sms.length > 0) {
                // save incoming sms
                saveIncomingSmsInfo(context, sms);
            }
        }
    }
    private void saveIncomingSmsInfo(final Context context, final SmsMessage[] sms) {
        Log.d(LOG_TAG, "saveIncomingSmsInfo()...");
        new Thread(new Runnable() {
            @Override
            public void run() {
                Log.d(LOG_TAG, "run()...");
                // save incoming sms
                SmsMessage smsMsg = sms[0];
                if (containedInBlacklist(context, smsMsg)) {
                    // Use now for the timestamp to avoid confusion with clock
                    // drift between the handset and the SMSC.
                    // Check to make sure the system is giving us a non-bogus time.
                    Calendar buildDate = new GregorianCalendar(2011, 8, 18);    // 18 Sep 2011
                    Calendar nowDate = new GregorianCalendar();
                    long now = System.currentTimeMillis();
                    nowDate.setTimeInMillis(now);
                    if (nowDate.before(buildDate)) {
                        // It looks like our system clock isn't set yet because the current time right now
                        // is before an arbitrary time we made this build. Instead of inserting a bogus
                        // receive time in this case, use the timestamp of when the message was sent.
                        now = smsMsg.getTimestampMillis();
                    }
                    int pduCount = sms.length;
                    StringBuilder body = new StringBuilder();
                    for (int i = 0; i < pduCount; i++) {
                        smsMsg = sms[i];
                        if (smsMsg.getMessageBody() != null) {
                            body.append(smsMsg.getMessageBody());
                        }
                    }

                    storeMessage(context,
                            smsMsg.getOriginatingAddress(),
                            now,
                            smsMsg.getTimestampMillis(),
                            smsMsg.getProtocolIdentifier(),
                            0,
                            smsMsg.isReplyPathPresent(),
                            smsMsg.getPseudoSubject(),
                            body.toString(),
                            smsMsg.getServiceCenterAddress(),
                            DBConfig.SystemSms.MESSAGE_TYPE_INBOX);
                }

            }
        }).start();
    }

    /*
     * Check number of new SMS is contained in BlackList
     */
    private boolean containedInBlacklist(Context context, SmsMessage msg) {
        String number = (msg != null ? msg.getOriginatingAddress() : null);
        if (TextUtils.isEmpty(number)) {
            return false;
        }
        ContentResolver cr = context.getContentResolver();
        String selection = String.format("%s AND (%s=%d OR %s=%d)",
                matchPhoneNumberCondition(number, DBConfig.BlackListInfo.NUMBER),
                DBConfig.BlackListInfo.INTERCEPT_TYPE, DBConfig.BlackListInfo.BLOCK_TYPE_ALL,
                DBConfig.BlackListInfo.INTERCEPT_TYPE, DBConfig.BlackListInfo.BLOCK_TYPE_SMS);
        boolean contained = false;
        Cursor cursor = null;
        try {
            cursor = cr.query(DBConfig.BlackListInfo.CONTENT_URI, new String[] {DBConfig.BlackListInfo._ID},
                selection, null, null);
            contained = (cursor != null && cursor.getCount() != 0);
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return contained;
    }

    public static String matchPhoneNumberCondition(String number, String tableItem) {
        String where = "(" + tableItem + "='" + number + "'" +
                " OR PHONE_NUMBERS_EQUAL(" + tableItem + "," + number + "))";
        return where;
    }

    /*
     * save message
     */
    public static Uri storeMessage(Context context,
                                   String number,
                                   long date,
                                   long dateSent,
                                   int protocolIdentifier,
                                   int read,
                                   boolean replyPathPresent,
                                   String subject,
                                   String body,
                                   String serviceCenterAddress,
                                   int smsType) {

        // Store the message in the content provider.
        ContentValues values = new ContentValues();
        values.put(DBConfig.SmsInfo.NUMBER, number);
        values.put(DBConfig.SmsInfo.DATE, Long.valueOf(date));
        values.put(DBConfig.SmsInfo.DATE_SENT, Long.valueOf(dateSent));
        values.put(DBConfig.SmsInfo.PROTOCOL, protocolIdentifier);
        values.put(DBConfig.SmsInfo.READ, read);
        values.put(DBConfig.SmsInfo.REPLY_PATH_PRESENT, replyPathPresent ? 1 : 0);
        values.put(DBConfig.SmsInfo.SUBJECT, subject);
        values.put(DBConfig.SmsInfo.BODY, replaceFormFeeds(body.toString()));
        values.put(DBConfig.SmsInfo.SERVICE_CENTER, serviceCenterAddress);
        values.put(DBConfig.SmsInfo.SMSTYPE, smsType);
        // insert sms into db
        ContentResolver cr = context.getContentResolver();

        return cr.insert(DBConfig.SmsInfo.CONTENT_URI, values);
    }
    private static String replaceFormFeeds(String s) {
        // Some providers send formfeeds in their messages. Convert those formfeeds to newlines.
        return s == null ? "" : s.replace('\f', '\n');
    }

    public final static SmsMessage[] getMessagesFromIntent(Intent intent) {
        Log.d(LOG_TAG, "getMessagesFromIntent()...");
        Object[] messages = (Object[]) intent.getSerializableExtra("pdus");
        //String format = intent.getStringExtra(ConsUtils.SMS_RECEIVED_FORMAT);
        byte[][] pduObjs = new byte[messages.length][];

        for (int i = 0; i < messages.length; i++) {
            pduObjs[i] = (byte[]) messages[i];
        }
        byte[][] pdus = new byte[pduObjs.length][];
        int pduCount = pdus.length;
        SmsMessage[] msgs = new SmsMessage[pduCount];
        for (int i = 0; i < pduCount; i++) {
            pdus[i] = pduObjs[i];
            msgs[i] = SmsMessage.createFromPdu(pdus[i]);
        }
        Log.d(LOG_TAG, "getMessagesFromIntent()...count:" + msgs.length);
        return msgs;
    }
}

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

import java.text.SimpleDateFormat;
import java.util.Date;

import android.content.Context;
import android.database.Cursor;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CursorAdapter;
import android.widget.TextView;
import android.text.TextUtils.TruncateAt;
import android.util.Log;
import android.provider.BaseColumns;

public class SmsFireLogAdapter extends CursorAdapter{

    private static final String LOG_TAG = "SmsFireLogAdapter";


    private Context mContext;
    private LayoutInflater mFactory;
    // remember select item
    private static View selectView;
    private static long selectId = -1;

    public SmsFireLogAdapter(Context context,Cursor cursor) {
        super(context, cursor, true);

        mContext = context;
        mFactory = LayoutInflater.from(context);
        selectView = null;
        selectId = -1;

    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        if(!getCursor().moveToPosition(position)){
            return null;
        }
        View view;
        if(convertView == null){
            view = newView(mContext,getCursor(),parent);
        }else{
            view = convertView;
        }
        bindView(view,mContext,getCursor());
        return view;
    }

    private void setNewHolder(View view){
        final ViewHolder viewHolder = new ViewHolder();
        viewHolder.number = (TextView)view.findViewById(R.id.call_log_number);
        viewHolder.date = (TextView)view.findViewById(R.id.call_log_date);
        viewHolder.body = (TextView)view.findViewById(R.id.call_log_location);
        viewHolder.body.setVisibility(View.VISIBLE);
        view.setTag(viewHolder);
    }

    static class ViewHolder{
         TextView number;
         TextView date;
         TextView body;
    }

    @Override
    public Object getItem(int position) {
        return super.getItem(position);
    }

    @Override
    public long getItemId(int position) {
        return super.getItemId(position);
    }

    @Override
    public synchronized Cursor swapCursor(Cursor newCursor) {
        final Cursor c = super.swapCursor(newCursor);
        return c;
    }

    @Override
    public void bindView(View view, Context context, final Cursor cursor) {
        Object tag = view.getTag();
        if(tag == null){
            setNewHolder(view);
        }
        final ViewHolder holder = (ViewHolder)view.getTag();
        holder.number.setText(cursor.getString(cursor.getColumnIndex(DBConfig.SmsInfo.NUMBER)));
        String time = getTime(cursor.getLong(cursor.getColumnIndex(DBConfig.SmsInfo.DATE)));
        holder.date.setText(time);
        holder.body.setText(cursor.getString(cursor.getColumnIndex(DBConfig.SmsInfo.BODY)));

        // set cursor id
        Long id = Long.valueOf(cursor.getLong(cursor.getColumnIndex(BaseColumns._ID)));
        // set visibility of body
        // hide the items that own the same view
        if (id != selectId && selectView == view) {
            displayBody(view, false);
        }
        // show the select item
        if (id == selectId) {
            // when id equals, the view should equal
            // if not equals, this may happen when db cursor is updated
            if (selectView != view) {
                displayBody(selectView, false);
                setSelectItem(id, view);
            }
            displayBody(view, true);
        }
        view.setTag(holder);
    }

    public static void setSelectItem(long id, View view) {
        Log.d(LOG_TAG, "setSelectItem()...");
        selectId = id;
        selectView = view;
    }
    public static long getSelectId() {
        return selectId;
    }
    public static View getSelectView() {
        return selectView;
    }

    @Override
    public View newView(Context c, Cursor cursor, ViewGroup parent) {
        final View view = mFactory.inflate(R.layout.call_log_list_item, null);
        setNewHolder(view);
        return view;
    }

    public String getTime(long time){
        SimpleDateFormat format=new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        Date date=new Date(time);
        String timeTrue=format.format(date);
        return timeTrue;
    }

    public static void displayBody(View view, boolean show) {
        Log.d(LOG_TAG, "displayBody()...show:" + show);
        if (view != null) {
            TextView body = (TextView)view.findViewById(R.id.call_log_location);
            if (body != null) {
                body.setSingleLine(!show);
                body.setEllipsize(show ? null : TruncateAt.END);
            }
        }
    }
    public static void showOrHideSmsBody(View selectView) {
        Log.d(LOG_TAG, "showOrHideSmsBody()...");
        long id = getSelectId();
        View view = getSelectView();
        boolean equalItem = (view == selectView);
        // hide last item
        if (view != null && !equalItem) {
            displayBody(view, false);
        }
        // set current item
        // if equal item, hide and reset selected item
        // if not equal item, show and set selected item
        displayBody(selectView, !equalItem);
        setSelectItem(equalItem ? -1 : id, equalItem ? null : selectView);
    }
}

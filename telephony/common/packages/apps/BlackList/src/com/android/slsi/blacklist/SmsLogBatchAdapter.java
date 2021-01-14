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
import java.util.HashMap;

import android.content.Context;
import android.database.Cursor;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CursorAdapter;
import android.widget.TextView;
import android.provider.BaseColumns;


public class SmsLogBatchAdapter extends CursorAdapter{

    private Context mContext;
    private LayoutInflater mFactory;
    public static HashMap<Integer,Boolean> isSelected;

    public SmsLogBatchAdapter(Context context,Cursor cursor) {
        super(context, cursor, true);

        mContext = context;
        mFactory = LayoutInflater.from(context);
        isSelected = new HashMap<Integer, Boolean>();
        initData();

    }

    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
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
        viewHolder.number = (TextView)view.findViewById(R.id.call_log_batch_number);
        viewHolder.date = (TextView)view.findViewById(R.id.call_log_batch_date);
        viewHolder.body = (TextView)view.findViewById(R.id.call_log_batch_location);
        viewHolder.body.setVisibility(View.VISIBLE);
        viewHolder.checkbox = (CheckBox)view.findViewById(R.id.log_batch_cb);
        view.setTag(viewHolder);
    }

    static class ViewHolder{
        TextView number;
        TextView date;
        TextView body;
        CheckBox checkbox;
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
        long id = cursor.getLong(cursor.getColumnIndex(DBConfig.SmsInfo._ID));
        holder.checkbox.setChecked(isSelected.get((int)id));
    }

    @Override
    public View newView(Context c, Cursor cursor, ViewGroup parent) {
        final View view = mFactory.inflate(R.layout.call_log_batch_list_item, null);
        setNewHolder(view);
        return view;
    }

    private void initData() {
        while(getCursor().moveToNext()){
            long id = getCursor().getLong(getCursor().getColumnIndex(BaseColumns._ID));
            isSelected.put((int)id, false);
        }
    }

    public String getTime(long time){
        SimpleDateFormat format=new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        Date date=new Date(time);
        String timeTrue=format.format(date);
        return timeTrue;
    }
}

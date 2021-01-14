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

public class CallFireLogAdapter extends CursorAdapter{

    private Context mContext;
    private LayoutInflater mFactory;

    public CallFireLogAdapter(Context context,Cursor cursor) {
        super(context, cursor, true);

        mContext = context;
        mFactory = LayoutInflater.from(context);

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
        viewHolder.location = (TextView)view.findViewById(R.id.call_log_location);
        viewHolder.interceptType = (TextView)view.findViewById(R.id.call_log_type);
        view.setTag(viewHolder);
    }

    static class ViewHolder{
         TextView number;
         TextView date;
         TextView location;
         TextView interceptType;
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
        holder.number.setText(cursor.getString(cursor.getColumnIndex(DBConfig.CallLogInfo.NUMBER)));
        String time = getTime(cursor.getLong(cursor.getColumnIndex(DBConfig.CallLogInfo.DATE)));
        holder.date.setText(time);
        holder.location.setText(cursor.getString(cursor.getColumnIndex(DBConfig.CallLogInfo.LOCATION)));
        holder.interceptType.setText(mContext.getString(R.string.block_call));
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

}

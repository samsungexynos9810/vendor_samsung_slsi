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


public class BatchLogAdapter extends CursorAdapter{

    private Context mContext;
    private LayoutInflater mFactory;
    public static HashMap<Integer,Boolean> isSelected;

    public BatchLogAdapter(Context context,Cursor cursor) {
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
        viewHolder.location = (TextView)view.findViewById(R.id.call_log_batch_location);
        viewHolder.interceptType = (TextView)view.findViewById(R.id.call_log_batch_type);
        viewHolder.checkbox = (CheckBox)view.findViewById(R.id.log_batch_cb);
        view.setTag(viewHolder);
    }

    static class ViewHolder{
        TextView number;
        TextView date;
        TextView location;
        TextView interceptType;
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
        holder.number.setText(cursor.getString(cursor.getColumnIndex(DBConfig.CallLogInfo.NUMBER)));
        String time = getTime(cursor.getLong(cursor.getColumnIndex(DBConfig.CallLogInfo.DATE)));
        holder.date.setText(time);
        holder.location.setText(cursor.getString(cursor.getColumnIndex(DBConfig.CallLogInfo.LOCATION)));
        holder.interceptType.setText(mContext.getString(R.string.block_call));
        long id = cursor.getLong(cursor.getColumnIndex(DBConfig.CallLogInfo._ID));
        holder.checkbox.setChecked(isSelected.get((int)id));
    }

    @Override
    public View newView(Context c, Cursor cursor, ViewGroup parent) {
        final View view = mFactory.inflate(R.layout.call_log_batch_list_item, null);
        setNewHolder(view);
        return view;
    }

    private String getBlockType(int block_type){
        String result = null;
        if (block_type == DBConfig.BlackListInfo.BLOCK_TYPE_ALL) {
            result = mContext.getString(R.string.block_call)+"、"+
                    mContext.getString(R.string.block_sms);
        } else if (block_type == DBConfig.BlackListInfo.BLOCK_TYPE_SMS){
            result = mContext.getString(R.string.block_sms);
        } else if (block_type == DBConfig.BlackListInfo.BLOCK_TYPE_CALL){
                result = mContext.getString(R.string.block_call);
        }
        return result;
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

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

import java.util.HashMap;
import java.util.Set;

import android.content.Context;
import android.database.Cursor;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CursorAdapter;
import android.widget.TextView;
import android.util.Log;
import android.provider.BaseColumns;


public class BatchListAdapter extends CursorAdapter{

    private Context mContext;
    private LayoutInflater mFactory;
    public static HashMap<Integer,Boolean> isSelected;

    public BatchListAdapter(Context context,Cursor cursor) {
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
        viewHolder.name = (TextView)view.findViewById(R.id.batch_list_name);
        viewHolder.number = (TextView)view.findViewById(R.id.batch_list_number);
        viewHolder.interceptType = (TextView)view.findViewById(R.id.batch_intercept_type);
        viewHolder.checkbox = (CheckBox)view.findViewById(R.id.batch_cb);
        view.setTag(viewHolder);
    }

    static class ViewHolder{
         TextView name;
         TextView number;
         TextView interceptType;
         CheckBox checkbox;
         BlackList blackList;
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
        final BlackList blackList = new BlackList(cursor);
        Object tag = view.getTag();
        if(tag == null){
            setNewHolder(view);
        }
        final ViewHolder holder = (ViewHolder)view.getTag();
        holder.blackList = blackList;
        if (holder.blackList.name == null) {
            holder.name.setText(R.string.no_name);
            holder.number.setText(holder.blackList.phoneNo);
        } else {
            holder.name.setText(holder.blackList.name);
            holder.number.setText(holder.blackList.phoneNo);
        }
        holder.number.setText(holder.blackList.phoneNo);
        holder.interceptType.setText(getBlockType(holder.blackList.intercept_type)
                == null ? "" : getBlockType(holder.blackList.intercept_type));
        holder.checkbox.setChecked(isSelected.get((int)holder.blackList.id));

    }

    @Override
    public View newView(Context c, Cursor cursor, ViewGroup parent) {
        final View view = mFactory.inflate(R.layout.batch_list_item, null);
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
}

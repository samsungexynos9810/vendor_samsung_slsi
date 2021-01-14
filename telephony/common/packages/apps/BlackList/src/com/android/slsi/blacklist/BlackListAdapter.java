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
import android.database.Cursor;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CursorAdapter;
import android.widget.TextView;

public class BlackListAdapter extends CursorAdapter{

    private Context mContext;
    private LayoutInflater mFactory;

    public BlackListAdapter(Context context,Cursor cursor) {
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
        viewHolder.name = (TextView)view.findViewById(R.id.list_name);
        viewHolder.number = (TextView)view.findViewById(R.id.list_number);
        viewHolder.interceptType = (TextView)view.findViewById(R.id.intercept_type);
        view.setTag(viewHolder);
    }

    static class ViewHolder{
         TextView name;
         TextView number;
         TextView interceptType;
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
        holder.number.setText(holder.blackList.phoneNo);
        holder.name.setText(holder.blackList.name
                == null ? mContext.getString(R.string.no_name) : holder.blackList.name);
        holder.interceptType.setText(getBlockType(holder.blackList.intercept_type)
                == null ? "" : getBlockType(holder.blackList.intercept_type));
    }

    @Override
    public View newView(Context c, Cursor cursor, ViewGroup parent) {
        final View view = mFactory.inflate(R.layout.black_list_item, null);
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

}

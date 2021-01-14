package com.samsung.slsi.cnntlogger;

import android.content.Context;
import android.os.AsyncTask;

public class ConcurrentTask extends AsyncTask<String, Integer, String> {

    private ConcurrentTaskListener mListener;

    private final Context mContext;

    public ConcurrentTask(Context context, ConcurrentTaskListener listener) {
        mContext = context;
        this.mListener = listener;
    }

    public interface ConcurrentTaskListener {
        void taskCompleted();
    }

    @Override
    protected void onPreExecute() {
        super.onPreExecute();
    }

    @Override
    protected String doInBackground(String... params) {
        new CmdRunner().startScript(mContext, params[0], params[1], params[2], params[3]);
        return "";
    }

    @Override
    protected void onProgressUpdate(Integer... progress) {
    }

    @Override
    protected void onPostExecute(String msg) {
        if(this.mListener != null) {
            this.mListener.taskCompleted();
        }
    }
}

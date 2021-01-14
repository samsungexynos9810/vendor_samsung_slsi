/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.oem;

import java.io.PrintStream;
import java.io.PrintWriter;
import java.net.URISyntaxException;
import java.util.ArrayList;

import android.content.Context;
import android.content.Intent;
import android.os.ShellCommand;
import android.os.UserHandle;

import com.android.internal.os.BaseCommand;

public class Am extends BaseCommand {

    private int mUserId;
    private Context mContext;

    /**
     * Command-line entry point.
     *
     * @param args The command-line arguments
     */
    public static void main(String[] args) {
        (new Am()).run(args);
    }

    public void run(Context context, String args) {
     // TODO need to split params correctly
        // e.g., --es name "value is a sentence"
        boolean start = false;
        StringBuilder sb = new StringBuilder();
        ArrayList<String> tokens = new ArrayList<String>();
        for (int i = 0; i < args.length(); i++) {
            char ch = args.charAt(i);
            if (ch == '\"') {
                if (start) {
                    String ret = sb.toString().trim();
                    if (ret != null && ret.length() > 0) {
                        tokens.add(ret);
                    }
                    sb = new StringBuilder();
                }
                start = !start;
                continue;
            }
            else if (ch == ' ') {
                if (!start) {
                    String ret = sb.toString().trim();
                    if (ret != null && ret.length() > 0) {
                        tokens.add(ret);
                    }
                    sb = new StringBuilder();
                    continue;
                }
            }
            sb.append(ch);
        }

        if (sb.length() > 0)
            tokens.add(sb.toString());

        if (start) {
            // invalid double quotes pair
            return ;
        }

        String[] newArgs = new String[tokens.size()];
        tokens.toArray(newArgs);
        run(context, newArgs);
    }

    public void run(Context context, String[] args) {
        mContext = context;
        run(args);
    }

    @Override
    public void onShowUsage(PrintStream out) {
        PrintWriter pw = new PrintWriter(out);
        pw.println(
                "usage: am [subcommand] [options]\n" +
                "usage: am start [-D] [-N] [-W] [-P <FILE>] [--start-profiler <FILE>]\n" +
                "               [--sampling INTERVAL] [-R COUNT] [-S]\n" +
                "               [--track-allocation] [--user <USER_ID> | current] <INTENT>\n" +
                "       am startservice [--user <USER_ID> | current] <INTENT>\n" +
                "       am stopservice [--user <USER_ID> | current] <INTENT>\n" +
                "       am broadcast [--user <USER_ID> | all | current] <INTENT>\n" +
                "\n" +
                "am start: start an Activity.  Options are:\n" +
                "    -D: enable debugging\n" +
                "    -N: enable native debugging\n" +
                "    -W: wait for launch to complete\n" +
                "    --start-profiler <FILE>: start profiler and send results to <FILE>\n" +
                "    --sampling INTERVAL: use sample profiling with INTERVAL microseconds\n" +
                "        between samples (use with --start-profiler)\n" +
                "    -P <FILE>: like above, but profiling stops when app goes idle\n" +
                "    -R: repeat the activity launch <COUNT> times.  Prior to each repeat,\n" +
                "        the top activity will be finished.\n" +
                "    -S: force stop the target app before starting the activity\n" +
                "    --track-allocation: enable tracking of object allocations\n" +
                "    --user <USER_ID> | current: Specify which user to run as; if not\n" +
                "        specified then run as the current user.\n" +
                "    --stack <STACK_ID>: Specify into which stack should the activity be put." +
                "\n" +
                "am startservice: start a Service.  Options are:\n" +
                "    --user <USER_ID> | current: Specify which user to run as; if not\n" +
                "        specified then run as the current user.\n" +
                "\n" +
                "am stopservice: stop a Service.  Options are:\n" +
                "    --user <USER_ID> | current: Specify which user to run as; if not\n" +
                "        specified then run as the current user.\n" +
                "\n" +
                "am broadcast: send a broadcast Intent.  Options are:\n" +
                "    --user <USER_ID> | all | current: Specify which user to send to; if not\n" +
                "        specified then send to all users.\n" +
                "    --receiver-permission <PERMISSION>: Require receiver to hold permission.\n" +
                "\n" +
                "\n"
        );
        Intent.printIntentArgsHelp(pw, "");
        pw.flush();
    }

    @Override
    public void onRun() throws Exception {
        String op = nextArgRequired();

        if (op.equals("start")) {
            runStart();
        } else if (op.equals("startservice")) {
            runStartService();
        } else if (op.equals("stopservice")) {
            runStopService();
        } else if (op.equals("broadcast")) {
            sendBroadcast();
        } else {
            showError("Error: unknown command '" + op + "'");
        }
    }

    int parseUserArg(String arg) {
        int userId;
        if ("all".equals(arg)) {
            userId = UserHandle.USER_ALL;
        } else if ("current".equals(arg) || "cur".equals(arg)) {
            userId = UserHandle.USER_CURRENT;
        } else {
            userId = Integer.parseInt(arg);
        }
        return userId;
    }

    private Intent makeIntent(int defUser) throws URISyntaxException {
        mUserId = defUser;
        return Intent.parseCommandArgs(mArgs, new Intent.CommandOptionHandler() {
            @Override
            public boolean handleOption(String opt, ShellCommand cmd) {
                if (opt.equals("--user")) {
                    mUserId = parseUserArg(nextArgRequired());
                } else {
                    return false;
                }
                return true;
            }
        });
    }

    private void runStartService() throws Exception {
        Intent intent = makeIntent(UserHandle.USER_CURRENT);
        if (mUserId == UserHandle.USER_ALL) {
            System.err.println("Error: Can't start activity with user 'all'");
            return;
        }
        System.out.println("Starting service: " + intent);
        try {
            mContext.startService(intent);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void runStopService() throws Exception {
        Intent intent = makeIntent(UserHandle.USER_CURRENT);
        if (mUserId == UserHandle.USER_ALL) {
            System.err.println("Error: Can't stop activity with user 'all'");
            return;
        }
        System.out.println("Stopping service: " + intent);
        try {
            mContext.stopService(intent);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void runStart() throws Exception {
        Intent intent = makeIntent(UserHandle.USER_CURRENT);

        if (mUserId == UserHandle.USER_ALL) {
            System.err.println("Error: Can't start service with user 'all'");
            return;
        }

        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        try {
            mContext.startActivity(intent);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void sendBroadcast() throws Exception {
        Intent intent = makeIntent(UserHandle.USER_CURRENT);
        try {
            mContext.sendBroadcast(intent);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }
}
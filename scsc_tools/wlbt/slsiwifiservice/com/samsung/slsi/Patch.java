package com.samsung.slsi;
import java.io.*;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.channels.FileChannel;
import java.util.List;
import java.util.stream.Stream;
import java.util.stream.Collectors;

public class Patch {
    public static String readLineFromFile(RandomAccessFile file) {
        try { 
            return file.readLine(); 
        }
        catch(IOException ex) { 
            return null; 
        }
    }

    public static boolean writeLineToFile(RandomAccessFile file, final String line) {
        try{ 
            file.writeBytes(line); 
            return true;
        }
        catch(IOException ex) { 
            return false; 
        }
    }

    public static boolean hasLineInFile(final String filename, final String line) {
        RandomAccessFile file = null;
        String str = null;
        try { 
            file = new RandomAccessFile(new File(filename), "r"); 
        }
        catch(FileNotFoundException ex) {
            /**
             * SystemServer.java is not found, which means that we cannot patch.
             * return false;
             */
            return false;
        }
        while((str = readLineFromFile(file)) != null) {
            if(str.indexOf(line) > 0) {
                /**
                 * SystemServer.java is already patched.
                 * return false;
                 */
                try{ 
                    file.close(); 
                }
                catch(IOException ex) { 
                    System.out.println(ex); 
                }
                return true;
            }
        }
        try{ 
            file.close(); 
        }
        catch(IOException ex) { 
            System.out.println(ex); 
        }
        return false;
    }

    public static final String SYSTEM_SERVER_PATCH = 
"                traceBeginAndSlog(\"SlsiWifiService\");" + "\n" +
"                mSystemServiceManager.startService(\"com.android.server.wifi.SlsiWifiService\");" + "\n" +
"                traceEnd();" + "\n";

    public static final String LOOPER_FIRST_PATCH = 
"    public interface HookInterface {" + "\n" + 
"        public boolean hook(boolean pre, Message msg);" + "\n" + 
"    };" + "\n" + 
"    private HookInterface mHookInterface;" + "\n" + 
"    public void setHookInterface(@Nullable HookInterface hook) {" + "\n" + 
"        mHookInterface = hook;" + "\n" +
"    }"+"\n";

    public static final String LOOPER_SECOND_PATCH = 
"            final HookInterface hook = me.mHookInterface;" + "\n" +
"            boolean drop = false;" + "\n" +
"            if (hook != null) {" + "\n" + 
"                drop = hook.hook(true, msg);" + "\n" + 
"            }" + "\n" + 
"            if(drop == true) {" + "\n" + 
"                msg.recycleUnchecked();" + "\n" + 
"                continue;" + "\n" +
"            }" + "\n";

    public static final String LOOPER_THIRD_PATCH = 
"            if (hook != null) {" + "\n" +
"                hook.hook(false, msg);" + "\n" +
"            }" + "\n";

    public static final String API_FIRST_PATCH = 
"    method public void setHookInterface(android.os.Looper.HookInterface);" + "\n";

    public static final String API_SECOND_PATCH = 
"  public static abstract interface Looper.HookInterface {" + "\n"+
"    method public abstract boolean hook(boolean, android.os.Message);" + "\n"+
"  }" + "\n";

    public static boolean needToPatchLooper() {
        String frameworkDir = System.getenv("ANDROID_BUILD_TOP");
        return !hasLineInFile(frameworkDir+"/frameworks/base/core/java/android/os/Looper.java", "HookInterface");
    }

    public static boolean needToPatchAPI(final String filename) {
        String frameworkDir = System.getenv("ANDROID_BUILD_TOP");
        return !hasLineInFile(frameworkDir+"/frameworks/base/api/"+filename, "HookInterface");
    }

    public static boolean needToPatchSystemServer() {
        String frameworkDir = System.getenv("ANDROID_BUILD_TOP");
        return !hasLineInFile(frameworkDir+"/frameworks/base/services/java/com/android/server/SystemServer.java", "SlsiWifiService");
    }

    public static boolean patchLooper() {
        String frameworkDir = System.getenv("ANDROID_BUILD_TOP");
        RandomAccessFile readFile = null;
        RandomAccessFile writeFile = null;
        String line = null;
        int patchTrigger = 0;
        boolean result = false;
        try {
            readFile = new RandomAccessFile(new File(frameworkDir+"/frameworks/base/core/java/android/os/Looper.java"), "r");
        }
        catch(FileNotFoundException ex) {
            return result;
        }
        try {
            writeFile = new RandomAccessFile(new File(frameworkDir+"/frameworks/base/core/java/android/os/Looper.java.new"), "rw");
        }
        catch(IOException ex) {
            try{ 
                readFile.close(); 
            }
            catch(IOException innerex) {
                System.out.println(innerex); 
            }
            return result;
        }
        while((line = readLineFromFile(readFile)) != null) {
            line = line+"\n";
            if(writeLineToFile(writeFile, line) == false) {
                break;
            }
            
            if(line.indexOf("private Printer mLogging;") > 0) {
                if(writeLineToFile(writeFile, LOOPER_FIRST_PATCH) == false) {
                    break;
                }
            }
            else if(line.indexOf(">>>>>") > 0) {
                patchTrigger = 2;
            }
            else if(line.indexOf("<<<<<") > 0) {
                patchTrigger = 3;
            }
            else if(patchTrigger == 2 && line.indexOf("}") > 0) {
                if(writeLineToFile(writeFile, LOOPER_SECOND_PATCH) == false) {
                    break;
                }
                patchTrigger = 0;
            }
            else if(patchTrigger == 3 && line.indexOf("}") > 0) {
                if(writeLineToFile(writeFile, LOOPER_THIRD_PATCH) == false) {
                    break;
                }
                patchTrigger = 0;
                result = true;
            }
        }
        if(writeFile != null) {
            try{ 
                writeFile.close();
            }
            catch(IOException ex) { 
                System.out.println(ex);
            }
        }
        if(readFile != null) {
            try{ 
                readFile.close(); 
            }
            catch(IOException ex) {
                System.out.println(ex);
            }
        }
        return result;
    }

    public static boolean patchSystemServer()
    {
        String frameworkDir = System.getenv("ANDROID_BUILD_TOP");
        RandomAccessFile readFile = null;
        RandomAccessFile writeFile = null;
        String line = null;
        int patchTrigger = 0;
        boolean result = false;
        try
        {
            readFile = new RandomAccessFile(new File(frameworkDir+"/frameworks/base/services/java/com/android/server/SystemServer.java"), "r");
        }
        catch(FileNotFoundException ex)
        {
            return result;
        }
        try
        {
            writeFile = new RandomAccessFile(new File(frameworkDir+"/frameworks/base/services/java/com/android/server/SystemServer.java.new"), "rw");
        }
        catch(IOException ex)
        {
            try{ readFile.close(); }
            catch(IOException innerex) { System.out.println(innerex); }
            return result;
        }
        while((line = readLineFromFile(readFile)) != null)
        {
            line = line+"\n";
            if(writeLineToFile(writeFile, line) == false)
            {
                break;
            }
            
            if(line.indexOf("traceBeginAndSlog(\"StartNsdService\");") > 0)
            {
                patchTrigger = 1;
            }
            else if(patchTrigger == 1 && line.indexOf("traceEnd();") > 0)
            {
                if(writeLineToFile(writeFile, SYSTEM_SERVER_PATCH) == false)
                {
                    break;
                }
                patchTrigger = 0;
                result = true;
            }
        }
        if(writeFile != null) {
            try{ writeFile.close(); }
            catch(IOException ex) { System.out.println(ex); }
        }
        if(readFile != null) {
            try{ readFile.close(); }
            catch(IOException ex) { System.out.println(ex); }
        }
        return result;
    }

    public static boolean patchAPI(final String filename)
    {
        String frameworkDir = System.getenv("ANDROID_BUILD_TOP");
        RandomAccessFile readFile = null;
        RandomAccessFile writeFile = null;
        String line = null;
        int patchTrigger = 0;
        boolean result = false;
        try
        {
            readFile = new RandomAccessFile(new File(frameworkDir+"/frameworks/base/api/"+filename), "r");
        }
        catch(FileNotFoundException ex)
        {
            return result;
        }
        try
        {
            writeFile = new RandomAccessFile(new File(frameworkDir+"/frameworks/base/api/"+filename+".new"), "rw");
        }
        catch(IOException ex)
        {
            try{ readFile.close(); }
            catch(IOException innerex) { System.out.println(innerex); }
            return result;
        }
        while((line = readLineFromFile(readFile)) != null)
        {
            line = line+"\n";
            if(writeLineToFile(writeFile, line) == false)
            {
                break;
            }
            if(line.indexOf("method public void setMessageLogging") > 0)
            {
                if(writeLineToFile(writeFile, API_FIRST_PATCH) == false)
                {
                    break;
                }
                patchTrigger = 2;
            }
            else if(patchTrigger == 2 && line.indexOf("}") > 0)
            {
                if(writeLineToFile(writeFile, API_SECOND_PATCH) == false)
                {
                    break;
                }
                patchTrigger = 0;
                result = true;
            }
        }
        if(writeFile != null) {
            try{ writeFile.close(); }
            catch(IOException ex) { System.out.println(ex); }
        }
        if(readFile != null) {
            try{ readFile.close(); }
            catch(IOException ex) { System.out.println(ex); }
        }
        return result;
    }

    public static void main(String[] args) {
        String frameworkDir = System.getenv("ANDROID_BUILD_TOP");
        // 1. Patch Looper.java
        if(needToPatchLooper() == true)
        {
            patchLooper();
        }

        // 2. Patch api/current.txt
        if(needToPatchAPI("current.txt") == true)
        {
            patchAPI("current.txt");
        }
        if(needToPatchAPI("system-current.txt") == true)
        {
            patchAPI("system-current.txt");
        }
        if(needToPatchAPI("test-current.txt") == true)
        {
            patchAPI("test-current.txt");
        }
        
        // 3. Patch SystemServer
        if(needToPatchSystemServer() == true)
        {
            patchSystemServer();
        }

        // 4. Apply all patches.
        try{
            final Runtime rt = Runtime.getRuntime();
            rt.exec("mv "+frameworkDir+"/frameworks/base/services/java/com/android/server/SystemServer.java.new"+" "+frameworkDir+"/frameworks/base/services/java/com/android/server/SystemServer.java");
        } catch(IOException ex) { System.out.println(ex); }
        try{
            final Runtime rt = Runtime.getRuntime();
            rt.exec("mv "+frameworkDir+"/frameworks/base/core/java/android/os/Looper.java.new"+" "+frameworkDir+"/frameworks/base/core/java/android/os/Looper.java");
        } catch(IOException ex) { System.out.println(ex); }
        try{
            final Runtime rt = Runtime.getRuntime();
            rt.exec("mv "+frameworkDir+"/frameworks/base/api/current.txt.new"+" "+frameworkDir+"/frameworks/base/api/current.txt");
        } catch(IOException ex) { System.out.println(ex); }
        try{
            final Runtime rt = Runtime.getRuntime();
            rt.exec("mv "+frameworkDir+"/frameworks/base/api/system-current.txt.new"+" "+frameworkDir+"/frameworks/base/api/system-current.txt");
        } catch(IOException ex) { System.out.println(ex); }
        try{
            final Runtime rt = Runtime.getRuntime();
            rt.exec("mv "+frameworkDir+"/frameworks/base/api/test-current.txt.new"+" "+frameworkDir+"/frameworks/base/api/test-current.txt");
        } catch(IOException ex) { System.out.println(ex); }
        
        //5. Create symbolic links
        try{
            final Runtime rt = Runtime.getRuntime();
            rt.exec("unlink "+frameworkDir+"/frameworks/opt/net/wifi/service/java/com/android/server/wifi/SlsiWifiService.java");
        } catch(IOException ex) { System.out.println(ex); }
        try{
            final Runtime rt = Runtime.getRuntime();
            rt.exec("ln -s "+args[0]+"/SlsiWifiService.java"+" "+frameworkDir+"/frameworks/opt/net/wifi/service/java/com/android/server/wifi/SlsiWifiService.java");
        } catch(IOException ex) { System.out.println(ex); }
    }
}

/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.silentlogging;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import android.content.ContentResolver;
import android.content.Context;
import android.content.res.AssetManager;
import android.net.Uri;
import android.util.Log;

/**
 * Created by jin-h.shin on 2015-12-07.
 */
public class SilentLoggingProfile {

    private static final String TAG = "SilentLoggingProfile";
    private static File mFile = new File(SilentLoggingControlInterface.getInstance().getSlogPath());
    private static Context mContext;

    private static class SilentLoggingProfileLoader {
        public static SilentLoggingProfile sInstance = new SilentLoggingProfile();
    }

    public static SilentLoggingProfile getInstance() {
        return SilentLoggingProfileLoader.sInstance;
    }

    public static void setContext(Context context) {
        mContext = context;
    }

    public boolean sendRequest(byte[] SendbyteArray) {
        boolean ret = false;
        if (SilentLoggingControlInterface.getInstance().getOemService() == null) {
            Log.d(TAG, "OemService is null");
            return ret;
        }
        try {
            SilentLoggingControlInterface.getInstance().getOemService().sendProfile(SendbyteArray);
            Log.d(TAG, "Write to dmd");
            ret = true;
        } catch (Exception ex) {
            Log.d(TAG, "sendRequest ex : " + ex);
        }
        return ret;
    }

    public boolean sendDefaultProfile() {
        Log.d(TAG, "sendDefaultProfile");
        try {
            if (canProfileRead()) {
                Log.d(TAG, "sendDefaultProfile : can ProfileRead");
                if (!sendDMReqFromProfile())
                    return false;
            } else {
                Log.d(TAG, "sendDefaultProfile : can not ProfileRead");
                if (!sendRequest(SilentLoggingConstant.DEFAULT_PROFILE_1))
                    return false;
                if (!sendRequest(SilentLoggingConstant.DEFAULT_PROFILE_2))
                    return false;
                if (!sendRequest(SilentLoggingConstant.DEFAULT_PROFILE_3))
                    return false;
            }
        } catch (Exception ex) {
            Log.d(TAG, "sendDefaultProfile ex : " + ex);
            return false;
        }
        return true;
    }

    private void checkProfile() {
        try {
            boolean isFileExist = false;

            String sourceDir = SilentLoggingControlInterface.getInstance().getSlogPath();
            String filename = "startdm.nprf";
            File dir = new File(sourceDir);

            for (File f : dir.listFiles()) {
                if (f.getName().endsWith((".nprf"))) {
                    Log.d(TAG, "checkProfile() - getRootNodeFromProfile : Searched file :" + f.getName());
                    mFile = f;
                    isFileExist = true;
                    break;
                }
            }

            if (!isFileExist) {
                try {
                    final AssetManager assetManager = mContext.getAssets();
                    String strDefaultProfile = "NNEXT_PROFILE.nprf";
                    File saveFile = new File(SilentLoggingControlInterface.getInstance().getSlogPath() + "/" + strDefaultProfile);
                    Log.i(TAG, "saveFile : " + saveFile.getPath());

                    //InputStream inStream = getClass().getResourceAsStream("/assets/" + strDefaultProfile);
                    InputStream inStream = assetManager.open(strDefaultProfile);
                    BufferedInputStream buffInStream = new BufferedInputStream(inStream);

                    FileOutputStream fileOutStream = new FileOutputStream(saveFile);
                    BufferedOutputStream outStream = new BufferedOutputStream(fileOutStream);

                    int nRead = -1;
                    byte[] buffer = new byte[1024];
                    while ((nRead = buffInStream.read(buffer, 0, 1024)) != -1) {
                        outStream.write(buffer, 0, nRead);
                    }
                    outStream.flush();

                    try {
                        Thread.sleep(5000);
                    } catch (Exception e) {
                        Log.d(TAG, "Ecxception:" + e);
                    }

                    mFile = saveFile;

                    fileOutStream.close();
                    outStream.close();
                    inStream.close();
                    buffInStream.close();
                    Log.d(TAG, "Profile generation complete\n");
                } catch (Exception e) {
                    Log.d(TAG, "Profile generation error : " + e);
                }
            }
        } catch (Exception ex) {
            Log.d(TAG, "checkProfile is fail");
        }
    }

    private boolean canProfileRead() {
        ZipInputStream zinstream = null;
        try {
            checkProfile();
            String filename = "startdm.nprf";
            String strZipFileName = filename.substring(0, filename.lastIndexOf(".")) + ".zip";
            File newfile = new File(strZipFileName);
            boolean Rename = mFile.renameTo(newfile);
            Log.d(TAG, "ParseSelectDMMessageItems Renamed file :" + strZipFileName);

            zinstream = new ZipInputStream(new FileInputStream(mFile));
            ZipEntry zentry = zinstream.getNextEntry();
            while (!zentry.getName().endsWith("Select DM Message Items.xml")) {
                //Log.d(TAG, "getRootNodeFromProfile : Zip entry name : " + zentry.getName());
                zentry = zinstream.getNextEntry();
            }

        } catch (Exception ex) {
            Log.d(TAG, "canProfileRead() - getRootNodeFromProfile ex : " + ex.toString());
        } finally {
            if (zinstream != null) {
                try {
                    zinstream.close();
                } catch (Exception ex) {
                }
            }
        }
        return true;
    }

    public void changeProfile(Uri uri) {
        // profile delete
        String slogDir = SilentLoggingControlInterface.getInstance().getSlogPath();
        //String profileDir = Environment.getExternalStorageDirectory().toString() + SilentLoggingConstant.PROFILE_DIR;
        File sdir = new File(slogDir);
        File sourceFile = new File(uri.getPath());
        Log.i(TAG, "path : " + sourceFile.getPath() + " name : " + sourceFile.getName());

        for (File f : sdir.listFiles()) {
            if (f.getName().endsWith((".nprf"))) {
                Log.d(TAG, "[delete]Searched profile file :" + f.getName());
                f.delete();
            }
        }
        // new profile copy
        String strNewProfile = sourceFile.getName();
        ContentResolver contentResolver = mContext.getContentResolver();
        try {
            File saveFile = new File(SilentLoggingControlInterface.getInstance().getSlogPath() + "/" + strNewProfile);
            //File sourceFile = new File(Environment.getExternalStorageDirectory().toString() + SilentLoggingConstant.PROFILE_DIR + "/" + strNewProfile);
            //FileInputStream fileInputStream = new FileInputStream(sourceFile);
            FileInputStream fileInputStream = (FileInputStream) contentResolver.openInputStream(uri);
            BufferedInputStream buffInStream = new BufferedInputStream(fileInputStream);
            FileOutputStream fileOutStream = new FileOutputStream(saveFile);
            BufferedOutputStream outStream = new BufferedOutputStream(fileOutStream);

            int nRead = -1;
            byte[] buffer = new byte[1024];
            while ((nRead = buffInStream.read(buffer, 0, 1024)) != -1) {
                outStream.write(buffer, 0, nRead);
            }
            outStream.flush();
            try {
                Thread.sleep(1000);
            } catch (Exception e) {
                Log.d(TAG, "Ecxception:" + e);
            }
            fileOutStream.close();
            outStream.close();
            fileInputStream.close();
            buffInStream.close();
        } catch (Exception ex) {
            Log.e(TAG, "changeProfile is fail : " + ex);
        }
    }

    public String resetProfile() {
        String slogDir = SilentLoggingControlInterface.getInstance().getSlogPath();
        String deleteResult = "";
        File sdir = new File(slogDir);
        // delete profile
        for (File f : sdir.listFiles()) {
            if (f.getName().endsWith((".nprf"))) {
                Log.d(TAG, "[delete]Searched profile file :" + f.getName());
                deleteResult += f.getName() + " was deleted.\n";
                f.delete();
            }
        }
        if (deleteResult.equals("")) {
            deleteResult += "No Profile file in " + sdir.getPath() + " folder.\n";
        }
        return deleteResult;
    }

    private boolean sendDMReqFromProfile() {
        Log.d(TAG, "sendDMReqFromProfile");

        int delay = 50;

        if (!sendRequest(SilentLoggingConstant.mStartCommand))
            return false;
        delay(delay);

        if (!sendRequest(SilentLoggingConstant.DM_CHANNEL_UPDATE))
            return false;
        delay(delay);

        if (!sendComItemSelectRequest())
            return false;
        delay(delay);

        if (!sendLTEItemSelectRequest())
            return false;
        delay(delay);

        if (!sendHSPAItemSelectRequest())
            return false;
        delay(delay);

        if (!sendEDGEItemSelectRequest())
            return false;
        delay(delay);

        if (!sendCDMAItemSelectRequest())
            return false;
        delay(delay);

        if (!sendRequest(SilentLoggingConstant.UE_CONFIG_CHANNEL_LTE_1))
            return false;
        delay(delay);

        if (!sendRequest(SilentLoggingConstant.UE_CONFIG_CHANNEL_HEDGE_1))
            return false;
        delay(delay);

        if (!sendRequest(SilentLoggingConstant.UE_CONFIG_CHANNEL_LTE_2))
            return false;
        delay(delay);

        if (!sendRequest(SilentLoggingConstant.UE_CONFIG_CHANNEL_HEDGE_2))
            return false;
        delay(delay);

        if (!sendRequest(SilentLoggingConstant.UE_CONFIG_CHANNEL_LTE_3))
            return false;
        delay(delay);

        if (!sendRequest(SilentLoggingConstant.UE_CONFIG_CHANNEL_HEDGE_3))
            return false;
        delay(delay);

        if (!sendRequest(SilentLoggingConstant.DM_TRACE_TB_GET))
            return false;
        delay(delay);

        if (!sendRequest(SilentLoggingConstant.DM_TRACE_START_LTE))
            return false;
        delay(delay);

        if (!sendRequest(SilentLoggingConstant.DM_TRACE_START_HEDGE))
            return false;
        delay(delay);

        if (!sendRequest(SilentLoggingConstant.DM_ILM_TB_GET))
            return false;
        delay(delay);

        if (!sendRequest(SilentLoggingConstant.DM_ILM_MSG_TB_GET))
            return false;
        delay(delay);

        if (!sendRequest(SilentLoggingConstant.DM_ILM_START))
            return false;
        delay(delay);

        if (!sendRequest(SilentLoggingConstant.DM_ILM_TB_GET_2))
            return false;
        delay(delay);

        if (!sendRequest(SilentLoggingConstant.DM_COMD_ITEM_REF))
            return false;
        delay(delay);

        if (!sendRequest(SilentLoggingConstant.DM_LTE_ITEM_REF))
            return false;
        delay(delay);

        if (!sendRequest(SilentLoggingConstant.DM_COMB_ITEM_REF))
            return false;
        return true;
    }

    static boolean SelComItemAll = true, SelLTEItemAll = true, SelHSPAItemAll = true, SelEDGEItemAll = true, SelCDMAItemAll = true;
    static boolean DeSelComItemAll = false, DeSelLTEItemAll = false, DeSelHSPAItemAll = false, DeSelEDGEItemAll = false;
    static boolean COMMONINFO = false, COMMONBASICINFO = false, COMMONDATAINFO = false, COMMONSIGNALINFO = false;
    static byte bLTE_PHY_PHY_STATUS_INFO = 0x00,
            bLTE_PHY_CELL_SERCH_MEAS_INFO = 0x00,
            bLTE_PHY_NCELL_INFO = 0x00,
            bLTE_PHY_SYSTEM_INFO = 0x00,
            bLTE_PHY_CHAN_QUAL_INFO = 0x00,
            bLTE_PHY_PARAMETER_INFO = 0x00,
            bLTE_PHY_PHICH_INFO = 0x00,
            bLTE_L1_RF_INFO = 0x00,
            bLTE_L1_SYNC_INFO = 0x00,
            bLTE_L1_DOWNLINK_INFO = 0x00,
            bLTE_L1_UPLINK_INFO = 0x00,
            bLTE_L1_MEASUREMENT_CONFIG = 0x00,
            bLTE_L2_UL_SPECIFIC_PARAM = 0x00,
            bLTE_L2_DL_SCH_CONFIG = 0x00,
            bLTE_L2_UL_SCH_CONFIG = 0x00,
            bLTE_L2_TIME_ALIGNMENT_TIMER = 0x00,
            bLTE_L2_PHR_CONFIG = 0x00,
            bLTE_L2_PREAMBLE_INFO = 0x00,
            bLTE_L2_POWER_RAMPING_STEP = 0x00,
            bLTE_L2_RA_SUPERVISION_INFO = 0x00,
            bLTE_L2_MAX_HARQ_MSG3TX = 0x00,
            bLTE_L2_RACH_INFO = 0x00,
            bLTE_L2_RNIT_INFO = 0x00,
            bLTE_L2_UL_SYNC_STAT_INFO = 0x00,
            bLTE_L2_RB_INFO = 0x00,
            bLTE_L2_RLC_STATUS_INFO = 0x00, bLTE_L2_PDCP_UL_INFO = 0x00,
            bLTE_L2_PDCP_DL_INFO = 0x00,
            bLTE_RRC_SERVING_CELL_INFO = 0x00,
            bLTE_RRC_STATUS_VARIABLE_INFO = 0x00,
            bLTE_RRC_PEER_MSG_INFO = 0x00,
            bLTE_RRC_TIMER_INFO = 0x00,
            bLTE_RRC_ASN_VER_INFO = 0x00,
            bLTE_RRC_RACH_MSG_INFO = 0x00,
            bLTE_NAS_SIM_DATA_INFO = 0x00,
            bLTE_NAS_STATE_VARIABLE_INFO = 0x00,
            bLTE_NAS_L3_MM_MSG_INFO = 0x00,
            bLTE_NAS_PLMN_SELECTION_INFO = 0x00,
            bLTE_NAS_NAS_SECURITY_INFO = 0x00,
            bLTE_NAS_PDP_INFO = 0x00,
            bLTE_NAS_IP_INFO = 0x00,
            bLTE_NAS_L3_SM_MSG_INFO = 0x00,
            bLTE_DATA_THROUGHPUT_INFO = 0x00,
            bLTE_DATA_TIMING_INFO = 0x00,
            bLTE_DATA_HANDOVER_STAT_INFO = 0x00,
            bLTE_DATA_CALL_DROP_INFO = 0x00,
            bHSPA_GP_POWER_CONTROL_INFO = 0x00,
            bHSPA_GP_TRCH_BLER_INFO = 0x00,
            bHSPA_GP_FINGER_INFO = 0x00,
            bHSPA_GD_DPA_INFO = 0x00,
            bHSPA_GD_DAP_TX_INFO = 0x00,
            bHSPA_UL1_UMTS_RF_INFO = 0x00,
            bHSPA_UL1_SEARCH_INFO = 0x00,
            bHSPA_UL1_FREQ_SEARCH_INFO = 0x00,
            bHSPA_UL1_POWER_CONTROL_INFO = 0x00,
            bHSPA_UL1_OLPC_INFO = 0x00,
            bHSPA_UL1_MID_TYPE_INFO = 0x00,
            bHSPA_UL1_CELL_MEAS_INFO = 0x00,
            bHSPA_UL1_INTER_FREQ_MEAS_INFO = 0x00,
            bHSPA_UL1_INTER_RAT_MEAS_INFO = 0x00,
            bHSPA_UL1_INTERNAL_MEAS_INFO = 0x00,
            bHSPA_UL1_SERVING_CELL_INFO = 0x00,
            bHSPA_UL1_INTRA_FREQ_RESEL_INFO = 0x00,
            bHSPA_UL1_INTER_FREQ_RESEL_INFO = 0x00,
            bHSPA_URRC_STATUS_INFO = 0x00,
            bHSPA_URRC_RB_MAPPING_INFO = 0x00,
            bHSPA_URRC_NETWORK_INFO = 0x00,
            bHSPA_UUL_RACH_CONFIG_INFO = 0x00,
            bHSPA_UUL_UDPCH_CONFIG_INFO = 0x00,
            bHSPA_UUL_POWER_INFO = 0x00,
            bHSPA_UL2_UPDCP_CHAN_CONF_INFO = 0x00,
            bHSPA_UL2_RLC_AM_CHAN_STAT_INFO = 0x00,
            bHSPA_UL2_URLC_AM_CONFIG_INFO = 0x00,
            bHSPA_UL2_RLC_UM_CHAN_STAT_INFO = 0x00,
            bHSPA_UL2_URLC_UM_CONFIG_INFO = 0x00,
            bHSPA_UL2_URLC_TM_CONFIG_INFO = 0x00,
            bHSPA_UL2_WCDMA_MAC_INFO = 0x00,
            bHSPA_UL2_HS_MAC_INFO = 0x00,
            bHSPA_UL2_EUL_MAC_INFO = 0x00,
            bHSPA_UL2_EUL_MAC_STAT_INFO = 0x00,
            bHSPA_MM_GMM_INFO = 0x00,
            bEDGE_TIME_SLOT_INFO = 0x00,
            bEDGE_RLC_INFO = 0x00,
            bEDGE_S_CELL_MEAS_REPORT = 0x00,
            bEDGE_PHY_DEDICATED_STATE_INFO = 0x00,
            bEDGE_S_CELL_INFO = 0x00,
            bEDGE_N_CELL_INFO = 0x00,
            bEDGE_3G_N_CELL_INFO = 0x00,
            bEDGE_HANDOVER_INFO = 0x00,
            bEDGE_HANDOVER_HISTORY_INFO = 0x00,
            bEDGE_BASIC_INFO = 0x00,
            bEDGE_MEASUREMENT_INFO = 0x00,
            bEDGE_POWER_CONTROL_INFO = 0x00,
            bEDGE_THROUGHPUT_INFO = 0x00,
            bEDGE_SNP_LLC_INFO = 0x00,
            bEDGE_LLC_THROUGHPUT_INFO = 0x00,
            bEDGE_QOS_INFO = 0x00,
            bEDGE_MM_GMM_INFO = 0x00;

    private boolean sendComItemSelectRequest() {
        byte cbasicinfo = 0x00, cdatainfo = 0x00, csignalinfo = 0x00;
        if (COMMONINFO) {
            if (COMMONBASICINFO)
                cbasicinfo = 0x01;
            if (COMMONDATAINFO)
                cdatainfo = 0x01;
            if (COMMONSIGNALINFO)
                csignalinfo = 0x01;
        }

        if (SelComItemAll == true) {
            byte[] commoninforequest = {
                    0x7f, 0x0f, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, (byte) 0xa0, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, (byte) 0xff, 0x7e};
            if (!sendRequest(commoninforequest))
                return false;
        } else if (DeSelComItemAll == true) {
            byte[] commoninforequest = {
                    0x7f, 0x0f, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, (byte) 0xa0, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e};
            if (!sendRequest(commoninforequest))
                return false;
        } else {
            byte[] commoninforequest = {
                    0x7F, 0x15, 0x00, 0x00, 0x0e, 0x00, 0x00, (byte) 0xFF, (byte) 0xa0, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
                    cbasicinfo, 0x02, cdatainfo, 0x03, csignalinfo, 0x7E};
            if (!sendRequest(commoninforequest))
                return false;
        }
        return true;
    }

    private boolean sendLTEItemSelectRequest() {
        if (SelLTEItemAll) {
            byte[] lteinforequest = {
                    0x7f, 0x0f, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, (byte) 0xa0, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, (byte) 0xff, 0x7e};
            if (!sendRequest(lteinforequest))
                return false;
        } else if (DeSelLTEItemAll) {
            byte[] lteinforequest = {
                    0x7f, 0x0f, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, (byte) 0xa0, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e};
            if (!sendRequest(lteinforequest))
                return false;
        } else {
            byte[] lteinforequest = {
                    0x7F, 0x6B, 0x00, 0x00, 0x18, 0x00, 0x00, (byte) 0xFF, (byte) 0xa0, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x2E,
                    0x00, bLTE_PHY_PHY_STATUS_INFO,
                    0x01, bLTE_PHY_CELL_SERCH_MEAS_INFO,
                    0x02, bLTE_PHY_NCELL_INFO,
                    0x04, bLTE_PHY_SYSTEM_INFO,
                    0x05, bLTE_PHY_CHAN_QUAL_INFO,
                    0x06, bLTE_PHY_PARAMETER_INFO,
                    0x07, bLTE_PHY_PHICH_INFO,
                    0x10, bLTE_L1_RF_INFO,
                    0x11, bLTE_L1_SYNC_INFO,
                    0x12, bLTE_L1_DOWNLINK_INFO,
                    0x13, bLTE_L1_UPLINK_INFO,
                    0x18, bLTE_L1_MEASUREMENT_CONFIG,
                    0x30, bLTE_L2_UL_SPECIFIC_PARAM,
                    0x31, bLTE_L2_DL_SCH_CONFIG,
                    0x32, bLTE_L2_UL_SCH_CONFIG,
                    0x33, bLTE_L2_TIME_ALIGNMENT_TIMER,
                    0x34, bLTE_L2_PHR_CONFIG,
                    0x35, bLTE_L2_PREAMBLE_INFO,
                    0x36, bLTE_L2_POWER_RAMPING_STEP,
                    0x37, bLTE_L2_RA_SUPERVISION_INFO,
                    0x38, bLTE_L2_MAX_HARQ_MSG3TX,
                    0x39, bLTE_L2_RACH_INFO,
                    0x3A, bLTE_L2_RNIT_INFO,
                    0x3C, bLTE_L2_UL_SYNC_STAT_INFO,
                    0x40, bLTE_L2_RB_INFO,
                    0x41, bLTE_L2_RLC_STATUS_INFO,
                    0x42, bLTE_L2_PDCP_UL_INFO,
                    0x43, bLTE_L2_PDCP_DL_INFO,
                    0x50, bLTE_RRC_SERVING_CELL_INFO,
                    0x51, bLTE_RRC_STATUS_VARIABLE_INFO,
                    0x52, bLTE_RRC_PEER_MSG_INFO,
                    0x53, bLTE_RRC_TIMER_INFO,
                    0x54, bLTE_RRC_ASN_VER_INFO,
                    0x55, bLTE_RRC_RACH_MSG_INFO,
                    0x58, bLTE_NAS_SIM_DATA_INFO,
                    0x59, bLTE_NAS_STATE_VARIABLE_INFO,
                    0x5A, bLTE_NAS_L3_MM_MSG_INFO,
                    0x5B, bLTE_NAS_PLMN_SELECTION_INFO,
                    0x5C, bLTE_NAS_NAS_SECURITY_INFO,
                    0x5D, bLTE_NAS_PDP_INFO,
                    0x5E, bLTE_NAS_IP_INFO,
                    0x5F, bLTE_NAS_L3_SM_MSG_INFO,
                    0x60, bLTE_DATA_THROUGHPUT_INFO,
                    0x61, bLTE_DATA_TIMING_INFO,
                    0x62, bLTE_DATA_HANDOVER_STAT_INFO,
                    0x63, bLTE_DATA_CALL_DROP_INFO,
                    0x7E};
            if (!sendRequest(lteinforequest))
                return false;
        }
        return true;
    }

    private boolean sendHSPAItemSelectRequest() {
        if (SelHSPAItemAll) {
            byte[] hspainforequest = {
                    0x7f, 0x0f, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, (byte) 0xa0, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, (byte) 0xff, 0x7e};
            if (!sendRequest(hspainforequest))
                return false;
        } else if (DeSelHSPAItemAll) {
            byte[] hspainforequest = {
                    0x7f, 0x0f, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, (byte) 0xa0, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e};
            if (!sendRequest(hspainforequest))
                return false;
        } else {
            byte[] hspainforequest = {
                    0x7F, 0x55, 0x00, 0x00, 0x12, 0x00, 0x00, (byte) 0xFF, (byte) 0xa0, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x23,
                    0x00, bHSPA_GP_POWER_CONTROL_INFO,
                    0x01, bHSPA_GP_TRCH_BLER_INFO,
                    0x02, bHSPA_GP_FINGER_INFO,
                    0x03, bHSPA_GD_DPA_INFO,
                    0x05, bHSPA_GD_DAP_TX_INFO,
                    0x10, bHSPA_UL1_UMTS_RF_INFO,
                    0x11, bHSPA_UL1_SEARCH_INFO,
                    0x12, bHSPA_UL1_FREQ_SEARCH_INFO,
                    0x13, bHSPA_UL1_POWER_CONTROL_INFO,
                    0x14, bHSPA_UL1_OLPC_INFO,
                    0x16, bHSPA_UL1_MID_TYPE_INFO,
                    0x17, bHSPA_UL1_CELL_MEAS_INFO,
                    0x18, bHSPA_UL1_INTER_FREQ_MEAS_INFO,
                    0x19, bHSPA_UL1_INTER_RAT_MEAS_INFO,
                    0x1A, bHSPA_UL1_INTERNAL_MEAS_INFO,
                    0x1B, bHSPA_UL1_SERVING_CELL_INFO,
                    0x1C, bHSPA_UL1_INTRA_FREQ_RESEL_INFO,
                    0x1D, bHSPA_UL1_INTER_FREQ_RESEL_INFO,
                    0x20, bHSPA_URRC_STATUS_INFO,
                    0x21, bHSPA_URRC_RB_MAPPING_INFO,
                    0x22, bHSPA_URRC_NETWORK_INFO,
                    0x28, bHSPA_UUL_RACH_CONFIG_INFO,
                    0x29, bHSPA_UUL_UDPCH_CONFIG_INFO,
                    0x2A, bHSPA_UUL_POWER_INFO,
                    0x30, bHSPA_UL2_UPDCP_CHAN_CONF_INFO,
                    0x31, bHSPA_UL2_RLC_AM_CHAN_STAT_INFO,
                    0x32, bHSPA_UL2_URLC_AM_CONFIG_INFO,
                    0x33, bHSPA_UL2_RLC_UM_CHAN_STAT_INFO,
                    0x34, bHSPA_UL2_URLC_UM_CONFIG_INFO,
                    0x35, bHSPA_UL2_URLC_TM_CONFIG_INFO,
                    0x36, bHSPA_UL2_WCDMA_MAC_INFO,
                    0x37, bHSPA_UL2_HS_MAC_INFO,
                    0x38, bHSPA_UL2_EUL_MAC_INFO,
                    0x39, bHSPA_UL2_EUL_MAC_STAT_INFO,
                    0x3A, bHSPA_MM_GMM_INFO,
                    0x7E};
            if (!sendRequest(hspainforequest))
                return false;
        }
        return true;
    }

    private boolean sendEDGEItemSelectRequest() {
        if (SelEDGEItemAll) {
            byte[] edgeinforequest = {
                    0x7f, 0x0f, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, (byte) 0xa0, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, (byte) 0xff, 0x7e};
            if (!sendRequest(edgeinforequest))
                return false;
        } else if (DeSelEDGEItemAll) {
            byte[] edgeinforequest = {
                    0x7f, 0x0f, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, (byte) 0xa0, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e};
            if (!sendRequest(edgeinforequest))
                return false;
        } else {
            byte[] edgeinforequest = {
                    0x7F, 0x31, 0x00, 0x00, 0x12, 0x00, 0x00, (byte) 0xFF, (byte) 0xa0, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x11,
                    0x00, bEDGE_TIME_SLOT_INFO,
                    0x01, bEDGE_RLC_INFO,
                    0x03, bEDGE_S_CELL_MEAS_REPORT,
                    0x04, bEDGE_PHY_DEDICATED_STATE_INFO,
                    0x05, bEDGE_S_CELL_INFO,
                    0x06, bEDGE_N_CELL_INFO,
                    0x07, bEDGE_3G_N_CELL_INFO,
                    0x08, bEDGE_HANDOVER_INFO,
                    0x09, bEDGE_HANDOVER_HISTORY_INFO,
                    0x0A, bEDGE_BASIC_INFO,
                    0x0B, bEDGE_MEASUREMENT_INFO,
                    0x0C, bEDGE_POWER_CONTROL_INFO,
                    0x0D, bEDGE_THROUGHPUT_INFO,
                    0x0E, bEDGE_SNP_LLC_INFO,
                    0x0F, bEDGE_LLC_THROUGHPUT_INFO,
                    0x10, bEDGE_QOS_INFO,
                    0x11, bEDGE_MM_GMM_INFO,
                    0x7E};
            if (!sendRequest(edgeinforequest))
                return false;
        }
        return true;
    }

    private boolean sendCDMAItemSelectRequest() {
        if (SelCDMAItemAll) {
            byte[] cdmainforequest = {
                    0x7f, 0x0f, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, (byte) 0xa0, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, (byte) 0xff, 0x7e};
            if (!sendRequest(cdmainforequest))
                return false;
        }
        return true;
    }

    public static int bitsetop(int byte_value, int pos) {
        byte_value = byte_value | (1 << pos); // To set a bit:
        return byte_value;
    }

    public static int bitunsetop(int byte_value, int pos) {
        byte_value = byte_value & ~(1 << pos); // To un-set a bit:
        return byte_value;
    }

    public static byte[] intToByteArray(int value) {
        byte[] b = new byte[4];
        for (int i = 0; i < 4; i++) {
            int offset = (b.length - 1 - i) * 8;
            b[i] = (byte) ((value >>> offset) & 0xFF);
        }
        return b;
    }

    private void delay(int delay) {
        try {
            Thread.sleep(delay);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}

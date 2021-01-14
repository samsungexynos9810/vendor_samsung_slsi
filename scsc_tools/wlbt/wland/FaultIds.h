/**
 * Copyright 2016 Samsung Electronics Co., Ltd.
 *
 * @file 
 *       git clone ssh://yuseok21.kim@scsc-gerrit:29418/Connectivity/FW/Wlan
 *       branch : dev
 *       path : mac/fault/
 *
 * Created by : yuseok21.kim
 * Update : SEP/10/2018
 */

#ifndef _FAULTIDS_H
#define _FAULTIDS_H

class FaultInd{

public:
    static FaultInd *mInstance;
    const char *getFaultMessage(int id);
    
    FaultInd();
    ~FaultInd();
    static FaultInd *getInstance();
};

#endif

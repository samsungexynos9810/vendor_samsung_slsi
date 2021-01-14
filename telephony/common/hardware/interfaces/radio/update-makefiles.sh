#!/bin/bash

options="-r vendor.samsung_slsi.telephony.hardware:vendor/samsung_slsi/telephony/common/hardware/interfaces \
         -r android.hidl:system/libhidl/transport \
         -r android.hardware:hardware/interfaces"

outputs="vendor/samsung_slsi/telephony/hardware/radio/1.0/default"
hidl-gen -Landroidbp $options vendor.samsung_slsi.telephony.hardware.radio@1.0;

outputs="vendor/samsung_slsi/telephony/hardware/radio/1.1/default"
hidl-gen -Landroidbp $options vendor.samsung_slsi.telephony.hardware.radio@1.1;

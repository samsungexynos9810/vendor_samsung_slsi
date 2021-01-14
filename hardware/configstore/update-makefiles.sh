#!/bin/bash

options="-r vendor.samsung_slsi.hardware:vendor/samsung_slsi/hardware \
         -r android.hidl:system/libhidl/transport \
         -r android.hardware:hardware/interfaces"

outputs="1.0/default"

hidl-gen -Landroidbp $options vendor.samsung_slsi.hardware.configstore@1.0;
#hidl-gen -Lc++-impl -o $outputs $options vendor.samsung_slsi.hardware.configstore@1.0::IExynosHWCConfigs;

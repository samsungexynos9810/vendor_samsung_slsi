#!/bin/bash

options="-r vendor.samsung_slsi.telephony.hardware:vendor/samsung_slsi/telephony/common/hardware/interfaces \
         -r android.hidl:system/libhidl/transport \
         -r android.hardware:hardware/interfaces"

outputs="vendor/samsung_slsi/telephony/hardware/radioExternal/1.0/default"

#hidl-gen -L c++-headers -o $outputs $options vendor.samsung_slsi.hardware.radioExternal@1.0;
#hidl-gen -Lmakefile $options vendor.samsung_slsi.hardware.radioExternal@1.0;
hidl-gen -Landroidbp $options vendor.samsung_slsi.telephony.hardware.radioExternal@1.0;
#hidl-gen -L androidbp-impl -o $outputs $options vendor.samsung_slsi.hardware.radioExternal@1.0;
#hidl-gen -L c++-impl -o $outputs $options vendor.samsung_slsi.hardware.radioExternal@1.0;

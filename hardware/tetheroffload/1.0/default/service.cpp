#define LOG_TAG "tetheroffload@1.0-service"

#include <hidl/LegacySupport.h>

#include "Tetheroffload.h"
#include "exynos-dit-ioctl.h"

using vendor::samsung_slsi::hardware::tetheroffload::V1_0::implementation::Tetheroffload;
using android::OK;
using android::status_t;

Tetheroffload *gHAL;

int main()
{
	gHAL = new Tetheroffload();
	if (nullptr == gHAL) return 0;

	gHAL->StartThreads();

	status_t status = gHAL->registerService();
	if (status != OK) {
		ALOGE("Cannot register Tetheroffload HAL service");
		return -1;
	}

	ALOGI("Tetheroffload HAL started");

	gHAL->ThreadJoin();

	ALOGI("Exit Tetheroffload HAL");

	return 0; // should never get here
}

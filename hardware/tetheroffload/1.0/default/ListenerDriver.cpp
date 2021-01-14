#define LOG_TAG "tetheroffload@1.0-service"

#include <hidl/LegacySupport.h>
#include <poll.h>

#include "Listener.h"
#include "exynos-dit-ioctl.h"


#define DEV_POLL_TIMEOUT		(-1)

void *drv_event_thread(void *arg)
{
	int fd;
	struct pollfd pfd;
	struct dit_cb_event event;

	arg = NULL;

	if ((fd = open(DEVICE_OFFLOAD, O_RDWR | O_CLOEXEC)) < 0) {
		ALOGE("drv_event_thread Failed opening %s\n", DEVICE_OFFLOAD);
		return nullptr;
	}

    pfd.fd = fd;
    pfd.events = POLLIN | POLLHUP;

	do {
        pfd.revents = 0;
        poll(&pfd, 1, DEV_POLL_TIMEOUT);

		if (pfd.revents & POLLIN) {
			int len;

			len = read(fd, &event, sizeof(struct dit_cb_event));
			ALOGI("received event %d\n", event.cb_event);
			gHAL->offload_callback_event(event.cb_event);
		}
	} while (1);

	ALOGI("%s exit", __func__);

	return nullptr;
}

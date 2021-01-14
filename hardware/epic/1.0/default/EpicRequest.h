#ifndef VENDOR_SAMSUNG_SLSI_HARDWARE_EPIC_V1_0_EPICREQUEST_H
#define VENDOR_SAMSUNG_SLSI_HARDWARE_EPIC_V1_0_EPICREQUEST_H

#include <vendor/samsung_slsi/hardware/epic/1.0/IEpicRequest.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <hidl/HidlSupport.h>

namespace vendor {
	namespace samsung_slsi {
		namespace hardware {
			namespace epic {
				namespace V1_0 {
					namespace implementation {

						using ::android::hardware::hidl_array;
						using ::android::hardware::hidl_memory;
						using ::android::hardware::hidl_string;
						using ::android::hardware::hidl_vec;
						using ::android::hardware::Return;
						using ::android::hardware::Void;
						using ::android::sp;

						struct EpicRequest : public IEpicRequest {
							EpicRequest();
							virtual ~EpicRequest();

							// Methods from ::vendor::samsung_slsi::hardware::epic::V1_0::IEpicRequest follow.
							Return<uint32_t> init(int32_t scenario_id) override;
							Return<uint32_t> init_multi(const hidl_vec<int32_t>& scenario_id_list) override;
							Return<uint32_t> acquire_lock() override;
							Return<uint32_t> release_lock() override;
							Return<uint32_t> acquire_lock_option(uint32_t value, uint32_t usec) override;
							Return<uint32_t> acquire_lock_multi_option(const hidl_vec<uint32_t>& value_list, const hidl_vec<uint32_t>& usec_list) override;
							Return<uint32_t> perf_hint(const hidl_string& name) override;
							Return<uint32_t> hint_release(const hidl_string& name) override;

							// Methods from ::android::hidl::base::V1_0::IBase follow.
							typedef void (*init_t)();
							typedef void (*term_t)();
							typedef int (*alloc_request_t)(int);
							typedef int (*alloc_multi_request_t)(const int[], int);
							typedef void (*free_request_t)(int);
							typedef bool (*acquire_t)(int);
							typedef bool (*release_t)(int);
							typedef bool (*acquire_option_t)(int, unsigned int, unsigned int);
							typedef bool (*acquire_multi_option_t)(int, const unsigned int [], const unsigned int [], int len);
							typedef bool (*hint_t)(int, const char *, ssize_t);
							typedef bool (*hint_release_t)(int, const char *, ssize_t);

							void *so_handle;
							int req_handle;

							init_t pfn_init;
							term_t pfn_term;
							alloc_request_t pfn_alloc_request;
							alloc_multi_request_t pfn_alloc_multi_request;
							free_request_t pfn_free_request;
							acquire_t pfn_acquire;
							acquire_option_t pfn_acquire_option;
							acquire_multi_option_t pfn_acquire_multi_option;
							hint_t pfn_hint;
							hint_release_t pfn_hint_release;
							release_t pfn_release;
						};

						// FIXME: most likely delete, this is only for passthrough implementations
						extern "C" IEpicRequest* HIDL_FETCH_IEpicRequest(const char* name);

					}  // namespace implementation
				}  // namespace V1_0
			}  // namespace epic
		}  // namespace hardware
	}  // namespace samsung_slsi
}  // namespace vendor

#endif  // VENDOR_SAMSUNG_SLSI_HARDWARE_EPIC_V1_0_EPICREQUEST_H

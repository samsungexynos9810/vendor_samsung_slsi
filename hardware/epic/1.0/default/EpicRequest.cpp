#include "EpicRequest.h"
#include <dlfcn.h>
#include <unistd.h>
#include <android/log.h>

namespace vendor {
	namespace samsung_slsi {
		namespace hardware {
			namespace epic {
				namespace V1_0 {
					namespace implementation {
						EpicRequest::EpicRequest() :
							so_handle(nullptr), req_handle(0)
						{
							if (sizeof(long) == sizeof(int))
								so_handle = dlopen("/vendor/lib/libepic_helper.so", RTLD_NOW);
							else
								so_handle = dlopen("/vendor/lib64/libepic_helper.so", RTLD_NOW);

							if (so_handle == nullptr) {
								pfn_init = nullptr;
								pfn_term = nullptr;
								pfn_alloc_request = nullptr;
								pfn_alloc_multi_request = nullptr;
								pfn_free_request = nullptr;
								pfn_acquire = nullptr;
								pfn_acquire_option = nullptr;
								pfn_acquire_multi_option = nullptr;
								pfn_release = nullptr;
								pfn_hint = nullptr;
								pfn_hint_release = nullptr;
								return;
							}

							pfn_init = (init_t)dlsym(so_handle, "epic_init");
							pfn_term = (term_t)dlsym(so_handle, "epic_term");
							pfn_alloc_request = (alloc_request_t)dlsym(so_handle, "epic_alloc_request");
							pfn_alloc_multi_request = (alloc_multi_request_t)dlsym(so_handle, "epic_alloc_multi_request");
							pfn_free_request = (free_request_t)dlsym(so_handle, "epic_free_request");
							pfn_acquire = (acquire_t)dlsym(so_handle, "epic_acquire");
							pfn_acquire_option = (acquire_option_t)dlsym(so_handle, "epic_acquire_option");
							pfn_acquire_multi_option = (acquire_multi_option_t)dlsym(so_handle, "epic_acquire_multi_option");
							pfn_hint = (hint_t)dlsym(so_handle, "epic_perf_hint");
							pfn_hint_release = (hint_t)dlsym(so_handle, "epic_hint_release");
							pfn_release = (release_t)dlsym(so_handle, "epic_release");

							if (pfn_init != nullptr)
								pfn_init();
						}

						EpicRequest::~EpicRequest()
						{
							if (req_handle == 0)
								return;

							if (pfn_free_request != nullptr)
								pfn_free_request(req_handle);

							if (pfn_term != nullptr)
								pfn_term();

							dlclose(so_handle);
						}

						// Methods from ::vendor::samsung_slsi::hardware::epic::V1_0::IEpicRequest follow.
						Return<uint32_t> EpicRequest::init(int32_t scenario_id) {
							if (pfn_alloc_request != nullptr)
								req_handle = pfn_alloc_request(scenario_id);
							return (req_handle != 0);
						}

						Return<uint32_t> EpicRequest::init_multi(const hidl_vec<int32_t>& scenario_id_list) {
							if (pfn_alloc_multi_request != nullptr)
								req_handle = pfn_alloc_multi_request(scenario_id_list.data(), scenario_id_list.size());
							return (req_handle != 0);
						}

						Return<uint32_t> EpicRequest::acquire_lock() {
							if (req_handle == 0 ||
								pfn_acquire == nullptr)
								return 0;

							return (uint32_t)pfn_acquire(req_handle);
						}

						Return<uint32_t> EpicRequest::release_lock() {
							if (req_handle == 0 ||
								pfn_release == nullptr)
								return 0;

							return (uint32_t)pfn_release(req_handle);
						}

						Return<uint32_t> EpicRequest::acquire_lock_option(uint32_t value, uint32_t usec) {
							if (req_handle == 0 ||
								pfn_acquire_option == nullptr)
								return 0;

							return (uint32_t)pfn_acquire_option(req_handle, value, usec);
						}

						Return<uint32_t> EpicRequest::acquire_lock_multi_option(const hidl_vec<uint32_t>& value_list, const hidl_vec<uint32_t>& usec_list) {
							if (req_handle == 0 ||
								pfn_acquire_multi_option == nullptr)
								return 0;

							return (uint32_t)pfn_acquire_multi_option(req_handle, value_list.data(), usec_list.data(), value_list.size());
						}

						Return<uint32_t> EpicRequest::perf_hint(const hidl_string& name) {
							if (req_handle == 0 ||
								pfn_hint == nullptr)
								return 0;

							return (uint32_t)pfn_hint(req_handle, name.c_str(), name.size());
						}

						Return<uint32_t> EpicRequest::hint_release(const hidl_string& name) {
							if (req_handle == 0 ||
								pfn_hint_release == nullptr)
								return 0;

							return (uint32_t)pfn_hint_release(req_handle, name.c_str(), name.size());
						}

						// Methods from ::android::hidl::base::V1_0::IBase follow.
						IEpicRequest* HIDL_FETCH_IEpicRequest(const char* /* name */) {
							return new EpicRequest();
						}
						//
					}  // namespace implementation
				}  // namespace V1_0
			}  // namespace epic
		}  // namespace hardware
	}  // namespace samsung_slsi
}  // namespace vendor

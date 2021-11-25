# To use stlport, "include device/fsl/mf0300_6dq/stlport/libstlport.mk" in your target.

# We put the STL libraries in front of any user libraries, but we need to
# keep the RTTI stuff in abi/cpp/include in front of our STL headers.
LOCAL_C_INCLUDES := \
	$(filter abi/cpp/include,$(LOCAL_C_INCLUDES)) \
	device/fsl/mf0300_6dq/stlport/stlport \
	bionic \
        bionic/libstdc++/include \
	$(filter-out abi/cpp/include,$(LOCAL_C_INCLUDES))

LOCAL_SHARED_LIBRARIES += libstlport

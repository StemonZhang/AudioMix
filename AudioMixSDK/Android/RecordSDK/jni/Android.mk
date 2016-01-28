LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := librecordsdk

LOCAL_C_INCLUDES :=  $(LOCAL_PATH)
LOCAL_C_INCLUDES +=  $(LOCAL_PATH)/src
LOCAL_C_INCLUDES +=  $(LOCAL_PATH)/faac
#LOCAL_C_INCLUDES += E:\ndk\android-ndk-r9d\platforms\android-18\arch-arm\usr\include

# Add your application source files here...
LOCAL_SRC_FILES := RecordSDKAPI.cpp \
./src/faacEncoder.cpp \
./src/MixAudio.cpp \
./src/PcmResample.cpp \
libfaac/aacquant.c \
                    libfaac/backpred.c  \
	           libfaac/bitstream.c  \
	           libfaac/channels.c \
	          libfaac/fft.c \
		  libfaac/filtbank.c \
		  libfaac/frame.c \
		   libfaac/huffman.c \
		   libfaac/ltp.c\
		   libfaac/midside.c \
		   libfaac/psychkni.c \
		   libfaac/tns.c      \
		   libfaac/util.c    \
		   libfaac/kiss_fft/kiss_fft.c \
		   libfaac/kiss_fft/kiss_fftr.c

#LOCAL_CFLAGS :=-DHAVE_CONFIG_H
LOCAL_LDLIBS := -L$(LOCAL_PATH)/faac -llog -lz -lstdc++

include $(BUILD_SHARED_LIBRARY)

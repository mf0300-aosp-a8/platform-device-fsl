/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "FicExtJni"
#include <utils/Log.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>

#include "jni.h"

#define _SERIAL_NUM_MAX_LEN			30
#define _SERIAL_NODE_NAME			"/sys/bus/i2c/drivers/at24/0-0050/eeprom"

static const char *classPathName = "com/android/server/FicExtCmd";

int _strlen(const char* p) {
	int l = 0;
	if (p == nullptr) 
		return l;
	while (*p != 0) {
		p++;
		l++;
	}
	return l;
}

static jstring getSerial(JNIEnv *env, jobject thiz) {
	jstring ret;
	int fd=open(_SERIAL_NODE_NAME, O_RDWR);
	if (0 == fd) {
		ALOGE("open device: %s failed\n", _SERIAL_NODE_NAME);
		return 0;
	}
	char szSerial[_SERIAL_NUM_MAX_LEN]={0};
	int nRead=static_cast<int>(read(fd, szSerial, _SERIAL_NUM_MAX_LEN));
	close(fd);
	ALOGE("res: %s, %d, %c\n", szSerial, nRead, szSerial[3]);
	//
	ret=env->NewStringUTF(szSerial);
	return ret;
}

static void setSerial(JNIEnv *env, jobject thiz, jstring cmd) {
	int fd=open(_SERIAL_NODE_NAME, O_RDWR);
	if (0 == fd) {
		ALOGE("open device: %s failed\n", _SERIAL_NODE_NAME);
		return ;
	}
	const char *szSerial = env->GetStringUTFChars(cmd, NULL);
	int nLeng = _strlen(szSerial);
	ALOGE("serial: %s, leng: %d\n", szSerial, nLeng);
	write(fd, szSerial, nLeng < _SERIAL_NUM_MAX_LEN ? nLeng : _SERIAL_NUM_MAX_LEN);
	env->ReleaseStringUTFChars(cmd, szSerial);
	close(fd);

}

static JNINativeMethod methods[] = {
  {"setSerial", "(Ljava/lang/String;)V", (void*)setSerial },
  {"getSerial", "()Ljava/lang/String;", (void*)getSerial },
};

/*
 * Register several native methods for one class.
 */
static int registerNativeMethods(JNIEnv* env, const char* className,
    JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    clazz = env->FindClass(className);
    if (clazz == NULL) {
        ALOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        ALOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

/*
 * Register native methods for all classes we know about.
 *
 * returns JNI_TRUE on success.
 */
static int registerNatives(JNIEnv* env)
{
  if (!registerNativeMethods(env, classPathName,
                 methods, sizeof(methods) / sizeof(methods[0]))) {
    return JNI_FALSE;
  }

  return JNI_TRUE;
}


// ----------------------------------------------------------------------------

/*
 * This is called by the VM when the shared library is first loaded.
 */
 
typedef union {
    JNIEnv* env;
    void* venv;
} UnionJNIEnvToVoid;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    jint result = -1;
    JNIEnv* env = NULL;
    
    ALOGI("JNI_OnLoad");

    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("ERROR: GetEnv failed");
        goto bail;
    }
    env = uenv.env;

    if (registerNatives(env) != JNI_TRUE) {
        ALOGE("ERROR: registerNatives failed");
        goto bail;
    }
    
    result = JNI_VERSION_1_4;
    
bail:
    return result;
}

#pragma once
// Minimal JNI ABI - table layout is frozen by the JNI spec, so declaring the
// prefix we use is safe. Avoids requiring a JDK on the build machine.
// Offsets follow JNINativeInterface_ / JavaVM_ exactly (JNI 1.1..1.8 order).
#include <cstdint>
#include <cstdarg>

// --- primitive typedefs ------------------------------------------------------
using jint    = int32_t;
using jlong   = int64_t;
using jbyte   = int8_t;
using jshort  = int16_t;
using jfloat  = float;
using jdouble = double;
using jchar   = uint16_t;
using jboolean= uint8_t;
using jsize   = jint;

struct _jobject;
using jobject  = _jobject*;
using jclass   = _jobject*;
using jstring  = _jobject*;
using jarray   = _jobject*;
using jobjectArray = _jobject*;
using jintArray= _jobject*;
using jbooleanArray = _jobject*;
using jbyteArray = _jobject*;
using jcharArray = _jobject*;
using jshortArray = _jobject*;
using jlongArray = _jobject*;
using jfloatArray = _jobject*;
using jdoubleArray = _jobject*;
using jthrowable = _jobject*;
using jweak    = _jobject*;
using jfieldID = struct _jfieldID*;
using jmethodID = struct _jmethodID*;

#ifndef JNICALL
#define JNICALL
#endif
#ifndef JNIEXPORT
#define JNIEXPORT
#endif
#ifndef JNI_OK
#define JNI_OK 0
#endif
#define JNI_VERSION_1_2 0x00010002
#define JNI_VERSION_1_4 0x00010004
#define JNI_VERSION_1_6 0x00010006
#define JNI_VERSION_1_8 0x00010008

union jvalue;

struct JavaVMAttachArgs {
    jint    version;
    char*   name;
    jobject group;
};

// The real jni.h passes attach args as void* - keep the ABI identical.

struct JNINativeInterface_ {
    void*       reserved0;              // 0
    void*       reserved1;              // 1
    void*       reserved2;              // 2
    void*       reserved3;              // 3
    jint        (JNICALL *GetVersion)(JNIEnv*);                                  // 4
    jclass      (JNICALL *DefineClass)(JNIEnv*, const char*, jobject, const jbyte*, jsize); // 5
    jclass      (JNICALL *FindClass)(JNIEnv*, const char*);                      // 6
    jmethodID   (JNICALL *FromReflectedMethod)(JNIEnv*, jobject);                // 7
    jfieldID    (JNICALL *FromReflectedField)(JNIEnv*, jobject);                 // 8
    jobject     (JNICALL *ToReflectedMethod)(JNIEnv*, jclass, jmethodID, jboolean); // 9
    jclass      (JNICALL *GetSuperclass)(JNIEnv*, jclass);                       // 10
    jboolean    (JNICALL *IsAssignableFrom)(JNIEnv*, jclass, jclass);            // 11
    jobject     (JNICALL *ToReflectedField)(JNIEnv*, jclass, jfieldID, jboolean);// 12
    jint        (JNICALL *Throw)(JNIEnv*, jthrowable);                           // 13
    jint        (JNICALL *ThrowNew)(JNIEnv*, jclass, const char*);               // 14
    jthrowable  (JNICALL *ExceptionOccurred)(JNIEnv*);                           // 15
    void        (JNICALL *ExceptionDescribe)(JNIEnv*);                           // 16
    void        (JNICALL *ExceptionClear)(JNIEnv*);                              // 17
    void        (JNICALL *FatalError)(JNIEnv*, const char*);                     // 18
    jint        (JNICALL *PushLocalFrame)(JNIEnv*, jint);                        // 19
    jobject     (JNICALL *PopLocalFrame)(JNIEnv*, jobject);                      // 20
    jobject     (JNICALL *NewGlobalRef)(JNIEnv*, jobject);                       // 21
    void        (JNICALL *DeleteGlobalRef)(JNIEnv*, jobject);                    // 22
    void        (JNICALL *DeleteLocalRef)(JNIEnv*, jobject);                     // 23
    jboolean    (JNICALL *IsSameObject)(JNIEnv*, jobject, jobject);              // 24
    jobject     (JNICALL *NewLocalRef)(JNIEnv*, jobject);                        // 25
    jint        (JNICALL *EnsureLocalCapacity)(JNIEnv*, jint);                   // 26
    jobject     (JNICALL *AllocObject)(JNIEnv*, jclass);                         // 27
    jobject     (JNICALL *NewObject)(JNIEnv*, jclass, jmethodID, ...);           // 28
    jobject     (JNICALL *NewObjectV)(JNIEnv*, jclass, jmethodID, va_list);      // 29
    jobject     (JNICALL *NewObjectA)(JNIEnv*, jclass, jmethodID, const jvalue*);// 30
    jclass      (JNICALL *GetObjectClass)(JNIEnv*, jobject);                     // 31
    jboolean    (JNICALL *IsInstanceOf)(JNIEnv*, jobject, jclass);               // 32
    jmethodID   (JNICALL *GetMethodID)(JNIEnv*, jclass, const char*, const char*);// 33
    jobject     (JNICALL *CallObjectMethod)(JNIEnv*, jobject, jmethodID, ...);   // 34
    jboolean    (JNICALL *CallBooleanMethod)(JNIEnv*, jobject, jmethodID, ...);  // 35
    jbyte       (JNICALL *CallByteMethod)(JNIEnv*, jobject, jmethodID, ...);     // 36
    jchar       (JNICALL *CallCharMethod)(JNIEnv*, jobject, jmethodID, ...);     // 37
    jshort      (JNICALL *CallShortMethod)(JNIEnv*, jobject, jmethodID, ...);    // 38
    jint        (JNICALL *CallIntMethod)(JNIEnv*, jobject, jmethodID, ...);      // 39
    jlong       (JNICALL *CallLongMethod)(JNIEnv*, jobject, jmethodID, ...);     // 40
    jfloat      (JNICALL *CallFloatMethod)(JNIEnv*, jobject, jmethodID, ...);    // 41
    jdouble     (JNICALL *CallDoubleMethod)(JNIEnv*, jobject, jmethodID, ...);   // 42
    void        (JNICALL *CallVoidMethod)(JNIEnv*, jobject, jmethodID, ...);     // 43
    jobject     (JNICALL *CallNonvirtualObjectMethod)(JNIEnv*, jobject, jclass, jmethodID, ...);  // 44
    jboolean    (JNICALL *CallNonvirtualBooleanMethod)(JNIEnv*, jobject, jclass, jmethodID, ...);// 45
    jbyte       (JNICALL *CallNonvirtualByteMethod)(JNIEnv*, jobject, jclass, jmethodID, ...);    // 46
    jchar       (JNICALL *CallNonvirtualCharMethod)(JNIEnv*, jobject, jclass, jmethodID, ...);    // 47
    jshort      (JNICALL *CallNonvirtualShortMethod)(JNIEnv*, jobject, jclass, jmethodID, ...);   // 48
    jint        (JNICALL *CallNonvirtualIntMethod)(JNIEnv*, jobject, jclass, jmethodID, ...);     // 49
    jlong       (JNICALL *CallNonvirtualLongMethod)(JNIEnv*, jobject, jclass, jmethodID, ...);    // 50
    jfloat      (JNICALL *CallNonvirtualFloatMethod)(JNIEnv*, jobject, jclass, jmethodID, ...);   // 51
    jdouble     (JNICALL *CallNonvirtualDoubleMethod)(JNIEnv*, jobject, jclass, jmethodID, ...);  // 52
    void        (JNICALL *CallNonvirtualVoidMethod)(JNIEnv*, jobject, jclass, jmethodID, ...);    // 53
    jfieldID    (JNICALL *GetFieldID)(JNIEnv*, jclass, const char*, const char*);// 54
    jobject     (JNICALL *GetObjectField)(JNIEnv*, jobject, jfieldID);           // 55
    jboolean    (JNICALL *GetBooleanField)(JNIEnv*, jobject, jfieldID);          // 56
    jbyte       (JNICALL *GetByteField)(JNIEnv*, jobject, jfieldID);             // 57
    jchar       (JNICALL *GetCharField)(JNIEnv*, jobject, jfieldID);             // 58
    jshort      (JNICALL *GetShortField)(JNIEnv*, jobject, jfieldID);            // 59
    jint        (JNICALL *GetIntField)(JNIEnv*, jobject, jfieldID);              // 60
    jlong       (JNICALL *GetLongField)(JNIEnv*, jobject, jfieldID);             // 61
    jfloat      (JNICALL *GetFloatField)(JNIEnv*, jobject, jfieldID);            // 62
    jdouble     (JNICALL *GetDoubleField)(JNIEnv*, jobject, jfieldID);           // 63
    void        (JNICALL *SetObjectField)(JNIEnv*, jobject, jfieldID, jobject);  // 64
    void        (JNICALL *SetBooleanField)(JNIEnv*, jobject, jfieldID, jboolean);// 65
    void        (JNICALL *SetByteField)(JNIEnv*, jobject, jfieldID, jbyte);      // 66
    void        (JNICALL *SetCharField)(JNIEnv*, jobject, jfieldID, jchar);      // 67
    void        (JNICALL *SetShortField)(JNIEnv*, jobject, jfieldID, jshort);    // 68
    void        (JNICALL *SetIntField)(JNIEnv*, jobject, jfieldID, jint);        // 69
    void        (JNICALL *SetLongField)(JNIEnv*, jobject, jfieldID, jlong);      // 70
    void        (JNICALL *SetFloatField)(JNIEnv*, jobject, jfieldID, jfloat);    // 71
    void        (JNICALL *SetDoubleField)(JNIEnv*, jobject, jfieldID, jdouble);  // 72
    jmethodID   (JNICALL *GetStaticMethodID)(JNIEnv*, jclass, const char*, const char*); // 73
    jobject     (JNICALL *CallStaticObjectMethod)(JNIEnv*, jclass, jmethodID, ...);  // 74
    jboolean    (JNICALL *CallStaticBooleanMethod)(JNIEnv*, jclass, jmethodID, ...); // 75
    jbyte       (JNICALL *CallStaticByteMethod)(JNIEnv*, jclass, jmethodID, ...);    // 76
    jchar       (JNICALL *CallStaticCharMethod)(JNIEnv*, jclass, jmethodID, ...);    // 77
    jshort      (JNICALL *CallStaticShortMethod)(JNIEnv*, jclass, jmethodID, ...);   // 78
    jint        (JNICALL *CallStaticIntMethod)(JNIEnv*, jclass, jmethodID, ...);     // 79
    jlong       (JNICALL *CallStaticLongMethod)(JNIEnv*, jclass, jmethodID, ...);    // 80
    jfloat      (JNICALL *CallStaticFloatMethod)(JNIEnv*, jclass, jmethodID, ...);   // 81
    jdouble     (JNICALL *CallStaticDoubleMethod)(JNIEnv*, jclass, jmethodID, ...);  // 82
    void        (JNICALL *CallStaticVoidMethod)(JNIEnv*, jclass, jmethodID, ...);    // 83
    jfieldID    (JNICALL *GetStaticFieldID)(JNIEnv*, jclass, const char*, const char*); // 84
    jobject     (JNICALL *GetStaticObjectField)(JNIEnv*, jclass, jfieldID);      // 85
    jboolean    (JNICALL *GetStaticBooleanField)(JNIEnv*, jclass, jfieldID);     // 86
    jbyte       (JNICALL *GetStaticByteField)(JNIEnv*, jclass, jfieldID);        // 87
    jchar       (JNICALL *GetStaticCharField)(JNIEnv*, jclass, jfieldID);        // 88
    jshort      (JNICALL *GetStaticShortField)(JNIEnv*, jclass, jfieldID);       // 89
    jint        (JNICALL *GetStaticIntField)(JNIEnv*, jclass, jfieldID);         // 90
    jlong       (JNICALL *GetStaticLongField)(JNIEnv*, jclass, jfieldID);        // 91
    jfloat      (JNICALL *GetStaticFloatField)(JNIEnv*, jclass, jfieldID);       // 92
    jdouble     (JNICALL *GetStaticDoubleField)(JNIEnv*, jclass, jfieldID);      // 93
    void        (JNICALL *SetStaticObjectField)(JNIEnv*, jclass, jfieldID, jobject);  // 94
    void        (JNICALL *SetStaticBooleanField)(JNIEnv*, jclass, jfieldID, jboolean);// 95
    void        (JNICALL *SetStaticByteField)(JNIEnv*, jclass, jfieldID, jbyte);      // 96
    void        (JNICALL *SetStaticCharField)(JNIEnv*, jclass, jfieldID, jchar);      // 97
    void        (JNICALL *SetStaticShortField)(JNIEnv*, jclass, jfieldID, jshort);    // 98
    void        (JNICALL *SetStaticIntField)(JNIEnv*, jclass, jfieldID, jint);        // 99
    void        (JNICALL *SetStaticLongField)(JNIEnv*, jclass, jfieldID, jlong);      // 100
    void        (JNICALL *SetStaticFloatField)(JNIEnv*, jclass, jfieldID, jfloat);    // 101
    void        (JNICALL *SetStaticDoubleField)(JNIEnv*, jclass, jfieldID, jdouble);  // 102
    jstring     (JNICALL *NewString)(JNIEnv*, const jchar*, jsize);              // 103
    jsize       (JNICALL *GetStringLength)(JNIEnv*, jstring);                    // 104
    const jchar*(JNICALL *GetStringChars)(JNIEnv*, jstring, jboolean*);          // 105
    void        (JNICALL *ReleaseStringChars)(JNIEnv*, jstring, const jchar*);   // 106
    jstring     (JNICALL *NewStringUTF)(JNIEnv*, const char*);                   // 107
    jsize       (JNICALL *GetStringUTFLength)(JNIEnv*, jstring);                 // 108
    const char* (JNICALL *GetStringUTFChars)(JNIEnv*, jstring, jboolean*);       // 109
    void        (JNICALL *ReleaseStringUTFChars)(JNIEnv*, jstring, const char*); // 110
    jsize       (JNICALL *GetArrayLength)(JNIEnv*, jarray);                      // 111
    jobjectArray(JNICALL *NewObjectArray)(JNIEnv*, jsize, jclass, jobject);      // 112
    jobject     (JNICALL *GetObjectArrayElement)(JNIEnv*, jobjectArray, jsize);  // 113
    void        (JNICALL *SetObjectArrayElement)(JNIEnv*, jobjectArray, jsize, jobject); // 114
    jbooleanArray (JNICALL *NewBooleanArray)(JNIEnv*, jsize);                    // 115
    jbyteArray    (JNICALL *NewByteArray)(JNIEnv*, jsize);                       // 116
    jcharArray    (JNICALL *NewCharArray)(JNIEnv*, jsize);                       // 117
    jshortArray   (JNICALL *NewShortArray)(JNIEnv*, jsize);                      // 118
    jintArray     (JNICALL *NewIntArray)(JNIEnv*, jsize);                        // 119
    jlongArray    (JNICALL *NewLongArray)(JNIEnv*, jsize);                       // 120
    jfloatArray   (JNICALL *NewFloatArray)(JNIEnv*, jsize);                      // 121
    jdoubleArray  (JNICALL *NewDoubleArray)(JNIEnv*, jsize);                     // 122
    jboolean*   (JNICALL *GetBooleanArrayElements)(JNIEnv*, jbooleanArray, jboolean*); // 123
    jbyte*      (JNICALL *GetByteArrayElements)(JNIEnv*, jbyteArray, jboolean*);       // 124
    jchar*      (JNICALL *GetCharArrayElements)(JNIEnv*, jcharArray, jboolean*);       // 125
    jshort*     (JNICALL *GetShortArrayElements)(JNIEnv*, jshortArray, jboolean*);     // 126
    jint*       (JNICALL *GetIntArrayElements)(JNIEnv*, jintArray, jboolean*);         // 127
    jlong*      (JNICALL *GetLongArrayElements)(JNIEnv*, jlongArray, jboolean*);       // 128
    jfloat*     (JNICALL *GetFloatArrayElements)(JNIEnv*, jfloatArray, jboolean*);     // 129
    jdouble*    (JNICALL *GetDoubleArrayElements)(JNIEnv*, jdoubleArray, jboolean*);   // 130
    void        (JNICALL *ReleaseBooleanArrayElements)(JNIEnv*, jbooleanArray, jboolean*, jint); // 131
    void        (JNICALL *ReleaseByteArrayElements)(JNIEnv*, jbyteArray, jbyte*, jint);          // 132
    void        (JNICALL *ReleaseCharArrayElements)(JNIEnv*, jcharArray, jchar*, jint);          // 133
    void        (JNICALL *ReleaseShortArrayElements)(JNIEnv*, jshortArray, jshort*, jint);       // 134
    void        (JNICALL *ReleaseIntArrayElements)(JNIEnv*, jintArray, jint*, jint);             // 135
    void        (JNICALL *ReleaseLongArrayElements)(JNIEnv*, jlongArray, jlong*, jint);          // 136
    void        (JNICALL *ReleaseFloatArrayElements)(JNIEnv*, jfloatArray, jfloat*, jint);       // 137
    void        (JNICALL *ReleaseDoubleArrayElements)(JNIEnv*, jdoubleArray, jdouble*, jint);    // 138
    void        (JNICALL *GetBooleanArrayRegion)(JNIEnv*, jbooleanArray, jsize, jsize, jboolean*); // 139
    void        (JNICALL *GetByteArrayRegion)(JNIEnv*, jbyteArray, jsize, jsize, jbyte*);         // 140
    void        (JNICALL *GetCharArrayRegion)(JNIEnv*, jcharArray, jsize, jsize, jchar*);         // 141
    void        (JNICALL *GetShortArrayRegion)(JNIEnv*, jshortArray, jsize, jsize, jshort*);      // 142
    void        (JNICALL *GetIntArrayRegion)(JNIEnv*, jintArray, jsize, jsize, jint*);            // 143
    void        (JNICALL *GetLongArrayRegion)(JNIEnv*, jlongArray, jsize, jsize, jlong*);         // 144
    void        (JNICALL *GetFloatArrayRegion)(JNIEnv*, jfloatArray, jsize, jsize, jfloat*);      // 145
    void        (JNICALL *GetDoubleArrayRegion)(JNIEnv*, jdoubleArray, jsize, jsize, jdouble*);   // 146
    void        (JNICALL *SetBooleanArrayRegion)(JNIEnv*, jbooleanArray, jsize, jsize, const jboolean*); // 147
    void        (JNICALL *SetByteArrayRegion)(JNIEnv*, jbyteArray, jsize, jsize, const jbyte*);    // 148
    void        (JNICALL *SetCharArrayRegion)(JNIEnv*, jcharArray, jsize, jsize, const jchar*);    // 149
    void        (JNICALL *SetShortArrayRegion)(JNIEnv*, jshortArray, jsize, jsize, const jshort*); // 150
    void        (JNICALL *SetIntArrayRegion)(JNIEnv*, jintArray, jsize, jsize, const jint*);       // 151
    void        (JNICALL *SetLongArrayRegion)(JNIEnv*, jlongArray, jsize, jsize, const jlong*);    // 152
    void        (JNICALL *SetFloatArrayRegion)(JNIEnv*, jfloatArray, jsize, jsize, const jfloat*); // 153
    void        (JNICALL *SetDoubleArrayRegion)(JNIEnv*, jdoubleArray, jsize, jsize, const jdouble*); // 154
    jint        (JNICALL *RegisterNatives)(JNIEnv*, jclass, const void*, jint);  // 155
    jint        (JNICALL *UnregisterNatives)(JNIEnv*, jclass);                   // 156
    jint        (JNICALL *MonitorEnter)(JNIEnv*, jobject);                       // 157
    jint        (JNICALL *MonitorExit)(JNIEnv*, jobject);                        // 158
    jint        (JNICALL *GetJavaVM)(JNIEnv*, JavaVM**);                         // 159
    void        (JNICALL *GetStringRegion)(JNIEnv*, jstring, jsize, jsize, jchar*);        // 160
    void        (JNICALL *GetStringUTFRegion)(JNIEnv*, jstring, jsize, jsize, char*);      // 161
    void*       (JNICALL *GetPrimitiveArrayCritical)(JNIEnv*, jarray, jboolean*); // 162
    void        (JNICALL *ReleasePrimitiveArrayCritical)(JNIEnv*, jarray, void*, jint); // 163
    const jchar*(JNICALL *GetStringCritical)(JNIEnv*, jstring, jboolean*);       // 164
    void        (JNICALL *ReleaseStringCritical)(JNIEnv*, jstring, const jchar*);// 165
    jweak       (JNICALL *NewWeakGlobalRef)(JNIEnv*, jobject);                   // 166
    void        (JNICALL *DeleteWeakGlobalRef)(JNIEnv*, jweak);                  // 167
    jboolean    (JNICALL *ExceptionCheck)(JNIEnv*);                              // 168
    jobject     (JNICALL *NewDirectByteBuffer)(JNIEnv*, void*, jlong);           // 169
    void*       (JNICALL *GetDirectBufferAddress)(JNIEnv*, jobject);             // 170
    jlong       (JNICALL *GetDirectBufferCapacity)(JNIEnv*, jobject);            // 171
    jobject     (JNICALL *GetObjectRefType)(JNIEnv*, jobject);                   // 172
    // nothing beyond 172 is used by this project
};

#include <cstdarg> // va_list

struct JavaVM_ {
    void*   reserved0;
    void*   reserved1;
    void*   reserved2;
    jint    (JNICALL *DestroyJavaVM)(JavaVM*);
    jint    (JNICALL *AttachCurrentThread)(JavaVM*, JNIEnv**, void*);
    jint    (JNICALL *DetachCurrentThread)(JavaVM*);
    jint    (JNICALL *GetEnv)(JavaVM*, void**, jint);
    jint    (JNICALL *AttachCurrentThreadAsDaemon)(JavaVM*, JNIEnv**, void*);
};

using JavaVM = JavaVM_;
using JNIEnv = JNINativeInterface_*;   // JNIEnv is a pointer to the table

// JNI_GetCreatedJavaVMs is looked up dynamically from jvm.dll at runtime.
extern "C" {
typedef jint (JNICALL *fn_JNI_GetCreatedJavaVMs)(JavaVM**, jsize, jsize*);
}

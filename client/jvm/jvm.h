#pragma once
// JVM attach layer: find jvm.dll, get the created VM, attach our threads.
#include "client/jvm/jni_min.h"
#include <optional>
#include <string>

namespace jvm {

bool init();       // locate jvm.dll + JNI_GetCreatedJavaVMs + GetEnv/attach
void shutdown();   // detach our thread

// Per-frame env for the render thread. nullptr if attach failed.
JNIEnv* env();

// Attach the calling thread (idempotent); returns nullptr on failure.
JNIEnv* attach_current_thread();

// Exception-check + clear after a JNI call. Returns true if an exception
// was pending (it has been cleared and logged when `where` is provided).
bool check_exc(JNIEnv* e, const char* where = nullptr);

// Scoped local-frame: PushLocalFrame on create, PopLocalFrame on destroy.
struct Frame {
    JNIEnv* e;
    bool ok = false;
    explicit Frame(JNIEnv* e_, jint capacity = 32);
    ~Frame();
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;
};

// Read a jstring as UTF-8 (empty string on failure).
std::string to_utf8(JNIEnv* e, jstring s);

} // namespace jvm

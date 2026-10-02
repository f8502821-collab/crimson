#include "client/jvm/jvm.h"
#include "common/log.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <psapi.h>   // EnumProcessModules
#include <cstring>

namespace jvm {

static const char* TAG = "jvm";
static JavaVM*  g_vm = nullptr;
static DWORD    g_main_thread = 0;

bool init() {
    HMODULE h = ::GetModuleHandleW(L"jvm.dll");
    if (!h) {
        // worst case: not yet loaded under the expected name - scan modules
        HMODULE mods[512]{};
        DWORD needed = 0;
        if (EnumProcessModules(::GetCurrentProcess(), mods, sizeof(mods), &needed)) {
            const int n = (int)(needed / sizeof(HMODULE));
            for (int i = 0; i < n && i < 512; ++i) {
                wchar_t name[MAX_PATH]{};
                GetModuleFileNameW(mods[i], name, MAX_PATH);
                // ends with jvm.dll?
                size_t len = wcslen(name);
                if (len >= 7 && _wcsicmp(name + len - 7, L"jvm.dll") == 0) { h = mods[i]; break; }
            }
        }
    }
    if (!h) {
        clog::error(TAG, "jvm.dll not found in process");
        return false;
    }

    auto get_created = reinterpret_cast<fn_JNI_GetCreatedJavaVMs>(
        ::GetProcAddress(h, "JNI_GetCreatedJavaVMs"));
    if (!get_created) {
        clog::error(TAG, "JNI_GetCreatedJavaVMs missing");
        return false;
    }

    JavaVM* vms[8]{};
    jsize n = 0;
    if (get_created(vms, 8, &n) != JNI_OK || n < 1) {
        clog::error(TAG, "no created JVMs (n=%d)", (int)n);
        return false;
    }
    g_vm = vms[0];

    JNIEnv* e = attach_current_thread();
    if (!e) return false;

    g_main_thread = ::GetCurrentThreadId();
    clog::info(TAG, "attached, JNI %d.%d", e->GetVersion(e) >> 16, e->GetVersion(e) & 0xFFFF);
    return true;
}

JNIEnv* attach_current_thread() {
    if (!g_vm) return nullptr;
    JNIEnv* e = nullptr;
    jint r = g_vm->GetEnv(g_vm, (void**)&e, JNI_VERSION_1_8);
    if (r == JNI_EDETACHED) {
        JavaVMAttachArgs a{ JNI_VERSION_1_8, (char*)"crimson", nullptr };
        r = g_vm->AttachCurrentThread(g_vm, (void**)&e, &a);
        if (r != JNI_OK) {
            clog::error(TAG, "AttachCurrentThread failed r=%d", (int)r);
            return nullptr;
        }
    } else if (r != JNI_OK) {
        clog::error(TAG, "GetEnv failed r=%d", (int)r);
        return nullptr;
    }
    return e;
}

void shutdown() {
    if (g_vm && ::GetCurrentThreadId() != g_main_thread)
        g_vm->DetachCurrentThread(g_vm);
}

bool check_exc(JNIEnv* e, const char* where) {
    if (!e) return false;
    if (e->ExceptionCheck(e)) {
        if (where) clog::error(TAG, "java exception pending at %s - cleared", where);
        e->ExceptionClear(e);
        return true;
    }
    return false;
}

Frame::Frame(JNIEnv* e_) : e(e_) {
    ok = e && e->PushLocalFrame(e, 32) == JNI_OK;
}
Frame::~Frame() {
    if (ok) e->PopLocalFrame(e, nullptr);
}

std::string to_utf8(JNIEnv* e, jstring s) {
    if (!e || !s) return {};
    const char* c = e->GetStringUTFChars(e, s, nullptr);
    if (!c) return {};
    std::string out(c);
    e->ReleaseStringUTFChars(e, s, c);
    return out;
}

JNIEnv* env() { return attach_current_thread(); }

} // namespace jvm

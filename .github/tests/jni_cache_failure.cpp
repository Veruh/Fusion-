#include <cmath>
#include "sdk/strayCache.h"
#include <iostream>
#include <string>

namespace {
int calls = 0;
int failAt = 0;
bool pending = false;
bool cleared = false;
bool fail() { ++calls; if (calls != failAt) return false; pending = true; return true; }
jmethodID JNICALL Method(JNIEnv*, jclass, const char*, const char*) { return fail() ? nullptr : reinterpret_cast<jmethodID>(1); }
jfieldID JNICALL Field(JNIEnv*, jclass, const char*, const char*) { return fail() ? nullptr : reinterpret_cast<jfieldID>(1); }
jboolean JNICALL HasException(JNIEnv*) { return pending ? JNI_TRUE : JNI_FALSE; }
void JNICALL ClearException(JNIEnv*) { pending = false; cleared = true; }
}

bool Java::AssignClass(std::string, jclass& out)
{
    if (fail()) { pending = false; out = nullptr; return false; }
    out = reinterpret_cast<jclass>(1);
    return true;
}
void Logger::Log(const char*, ...) {}

int main()
{
    JNINativeInterface_ table{};
    table.GetMethodID = Method;
    table.GetStaticMethodID = Method;
    table.GetFieldID = Field;
    table.GetStaticFieldID = Field;
    table.ExceptionCheck = HasException;
    table.ExceptionClear = ClearException;
    JNIEnv_ env{&table};
    Java::Env = &env;
    Java::Version = MinecraftVersion::LUNAR_1_8_9;

    for (int failed : {1, 2, 35, 121}) {
        calls = 0; failAt = failed; cleared = false; pending = false;
        StrayCache::initialized = false;
        StrayCache::Initialize();
        if (StrayCache::initialized || calls != failed || pending) return failed;
        if (failed == 1 && std::string(StrayCache::failureName) != "class: net.minecraft.client.Minecraft") return 125;
        if (failed == 2 && std::string(StrayCache::failureName) != "mapping: minecraft_getMinecraft") return 126;
        if (failed == 2 && !cleared) return 123;
    }
    calls = 0; failAt = 0; pending = false;
    StrayCache::Initialize();
    if (!StrayCache::initialized || calls != 121) return 124;
    std::cout << "JNI cache stops at simulated failures and initializes on success\n";
}


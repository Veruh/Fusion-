#include "java.h"
#include <util/logger.h>

#include "sdk/java/lang/String.h"
#include <algorithm>

JavaVM* vm;
jobject classLoader;
jmethodID mid_findClass;
static bool attachedByFusion = false;

bool setupClassLoader()
{
    if (Java::Env->PushLocalFrame(32) != JNI_OK) return false;
    auto fail = []() {
        if (Java::Env->ExceptionCheck()) Java::Env->ExceptionClear();
        Java::Env->PopLocalFrame(nullptr);
        return false;
    };
    jclass c_Thread = Java::Env->FindClass("java/lang/Thread");
    if (!c_Thread || Java::Env->ExceptionCheck()) return fail();
    jclass c_Map = Java::Env->FindClass("java/util/Map");
    if (!c_Map || Java::Env->ExceptionCheck()) return fail();
    jclass c_Set = Java::Env->FindClass("java/util/Set");
    if (!c_Set || Java::Env->ExceptionCheck()) return fail();
    jclass c_ClassLoader = Java::Env->FindClass("java/lang/ClassLoader");
    if (!c_ClassLoader || Java::Env->ExceptionCheck()) return fail();

    jmethodID mid_getAllStackTraces = Java::Env->GetStaticMethodID(c_Thread, "getAllStackTraces", "()Ljava/util/Map;");
    if (!mid_getAllStackTraces || Java::Env->ExceptionCheck()) return fail();
    jmethodID mid_keySet = Java::Env->GetMethodID(c_Map, "keySet", "()Ljava/util/Set;");
    if (!mid_keySet || Java::Env->ExceptionCheck()) return fail();
    jmethodID mid_toArray = Java::Env->GetMethodID(c_Set, "toArray", "()[Ljava/lang/Object;");
    if (!mid_toArray || Java::Env->ExceptionCheck()) return fail();
    jmethodID mid_getContextClassLoader = Java::Env->GetMethodID(c_Thread, "getContextClassLoader", "()Ljava/lang/ClassLoader;");
    if (!mid_getContextClassLoader || Java::Env->ExceptionCheck()) return fail();
    mid_findClass = Java::Env->GetMethodID(c_ClassLoader, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    if (!mid_findClass || Java::Env->ExceptionCheck()) return fail();

    jobject obj_stackTracesMap = Java::Env->CallStaticObjectMethod(c_Thread, mid_getAllStackTraces);
    if (!obj_stackTracesMap || Java::Env->ExceptionCheck()) return fail();
    jobject obj_threadsSet = Java::Env->CallObjectMethod(obj_stackTracesMap, mid_keySet);
    if (!obj_threadsSet || Java::Env->ExceptionCheck()) return fail();

    jobjectArray threads = (jobjectArray)Java::Env->CallObjectMethod(obj_threadsSet, mid_toArray);
    if (!threads || Java::Env->ExceptionCheck()) return fail();
    jint szThreads = Java::Env->GetArrayLength(threads);

    for (int i = 0; i < szThreads; i++)
    {
        jobject thread = Java::Env->GetObjectArrayElement(threads, i);
        if (!thread || Java::Env->ExceptionCheck()) return fail();
        jobject classLoaderObj = Java::Env->CallObjectMethod(thread, mid_getContextClassLoader);
        if (Java::Env->ExceptionCheck()) return fail();

        if (classLoaderObj) {
            
            jstring className = Java::Env->NewStringUTF("net.minecraft.client.Minecraft");
            if (!className || Java::Env->ExceptionCheck()) return fail();
            jobject minecraftClass = Java::Env->CallObjectMethod(classLoaderObj, mid_findClass, className);

            if (Java::Env->ExceptionCheck()) Java::Env->ExceptionClear();
            if (minecraftClass)
            {
                classLoader = Java::Env->NewGlobalRef(classLoaderObj);
                if (!classLoader || Java::Env->ExceptionCheck()) return fail();

                Java::Env->DeleteLocalRef(minecraftClass);
                Java::Env->DeleteLocalRef(classLoaderObj);
                Java::Env->DeleteLocalRef(className);
                Java::Env->DeleteLocalRef(thread);

                break;
            }
            Java::Env->DeleteLocalRef(className);
            Java::Env->DeleteLocalRef(classLoaderObj);
        }

        Java::Env->DeleteLocalRef(thread);
    }

    Java::Env->PopLocalFrame(nullptr);
    return classLoader != nullptr;
}

void Java::Init()
{
    Java::Initialized = false;
    Java::Version = MinecraftVersion::UNKNOWN;

    // Check if there is any Java VMs in the injected thread
    jsize count;
    if (JNI_GetCreatedJavaVMs(&vm, 1, &count) != JNI_OK || count == 0)
        return;

    jint res = vm->GetEnv((void**)&Java::Env, JNI_VERSION_1_6);
    Logger::Log("Got Java ENV");

    if (res == JNI_EDETACHED) {
        res = vm->AttachCurrentThread((void**)&Java::Env, nullptr);
		attachedByFusion = res == JNI_OK;
	}
	Logger::Log("Attached to Java VM");

    if (res != JNI_OK)
        return;

    if (Java::Env == nullptr)
        return;

    if (vm->GetEnv((void**)&Java::tiEnv, JVMTI_VERSION) != JNI_OK || !Java::tiEnv)
        return;
    if (!setupClassLoader()) return;
	Logger::Log("Java initialized");

    GetMinecraftVersion();
	Java::Initialized = Java::Version != MinecraftVersion::UNKNOWN;
	Logger::Log("Got Minecraft version");
}

void Java::Kill()
{
    if (Java::Env && classLoader) Java::Env->DeleteGlobalRef(classLoader);
    classLoader = nullptr;
    if (vm && attachedByFusion) vm->DetachCurrentThread();
    attachedByFusion = false;
    Java::Env = nullptr;
    Java::Initialized = false;
}

bool Java::AssignClass(std::string name, jclass &out)
{
    out = GetClass(name);
    return out != nullptr;
}

jclass Java::GetClass(std::string name)
{
    if (!Java::Env || !classLoader || !mid_findClass || name.empty()) return nullptr;
    jstring className = Java::Env->NewStringUTF(name.c_str());
    if (!className || Java::Env->ExceptionCheck()) {
        if (Java::Env->ExceptionCheck()) Java::Env->ExceptionClear();
        return nullptr;
    }
    jobject findClass = Java::Env->CallObjectMethod(classLoader, mid_findClass, className);
    Java::Env->DeleteLocalRef(className);
    if (Java::Env->ExceptionCheck()) Java::Env->ExceptionClear();

	if (findClass)
	{
		return (jclass)findClass;
	}

    std::replace(name.begin(), name.end(), '.', '/');
	jclass result = Java::Env->FindClass(name.c_str());
    if (Java::Env->ExceptionCheck()) Java::Env->ExceptionClear();
	return result;
}

jclass Java::findClass(JNIEnv* p_env, jvmtiEnv* p_tienv, const std::string& path)
{
    jint class_count = 0;
    jclass* classes = nullptr;
    jclass foundclass = nullptr;
    p_tienv->GetLoadedClasses(&class_count, &classes);
    for (int i = 0; i < class_count; ++i)
    {
        char* signature_buffer = nullptr;
        p_tienv->GetClassSignature(classes[i], &signature_buffer, nullptr);
        std::string signature = signature_buffer;
        p_tienv->Deallocate((unsigned char*)signature_buffer);
        signature = signature.substr(1);
        signature.pop_back();
        if (signature == path)
        {
            foundclass = (jclass)p_env->NewLocalRef(classes[i]);
        }
        p_env->DeleteLocalRef(classes[i]);
    }
    p_tienv->Deallocate((unsigned char*)classes);
    return foundclass;
}

static bool CheckLunarClient()
{
    jclass minecraftClass;
    Java::AssignClass("net.minecraft.client.Minecraft", minecraftClass);
    if (!minecraftClass) return false;

    jmethodID getMinecraftMethod = Java::Env->GetStaticMethodID(minecraftClass, "getMinecraft", "()Lnet/minecraft/client/Minecraft;");
    if (!getMinecraftMethod) return false;

    jobject theMinecraft = Java::Env->CallStaticObjectMethod(minecraftClass, getMinecraftMethod);
    if (!theMinecraft) return false;

    jfieldID launchedVersionField = Java::Env->GetFieldID(minecraftClass, "launchedVersion", "Ljava/lang/String;");
    if (!launchedVersionField) return false;

    jobject launchedVersion = Java::Env->GetObjectField(theMinecraft, launchedVersionField);
    if (!launchedVersion) return false;

    std::string version = String(launchedVersion).ToString();
	Logger::Log("Lunar: %s", version.c_str());
    if (version == "1.8.9") { Java::Version = MinecraftVersion::LUNAR_1_8_9; return true; }
    else if (version == "1.7.10") { Java::Version = MinecraftVersion::LUNAR_1_7_10; return true; }

    return false;
}

static bool CheckLabyMod()
{
    jclass minecraftClass;
    Java::AssignClass("net.labymod.api.client.Minecraft", minecraftClass);
    if (!minecraftClass) return false;

    jmethodID getMinecraftMethod = Java::Env->GetStaticMethodID(minecraftClass, "getMinecraft", "()Lnet/minecraft/client/Minecraft;");
    if (!getMinecraftMethod) return false;

    jobject theMinecraft = Java::Env->CallStaticObjectMethod(minecraftClass, getMinecraftMethod);
    if (!theMinecraft) return false;

    jfieldID launchedVersionField = Java::Env->GetFieldID(minecraftClass, "launchedVersion", "Ljava/lang/String;");
    if (!launchedVersionField) return false;

    jobject launchedVersion = Java::Env->GetObjectField(theMinecraft, launchedVersionField);
    if (!launchedVersion) return false;

    std::string version = String(launchedVersion).ToString();
    if (version == "1.8.9") { Java::Version = MinecraftVersion::LUNAR_1_8_9; return true; }
    else if (version == "1.7.10") { Java::Version = MinecraftVersion::LUNAR_1_7_10; return true; }

    return false;
}

static bool CheckVanilla189()
{
    jclass minecraftClass;
    Java::AssignClass("ave", minecraftClass);
    if (!minecraftClass) return false;

    jmethodID getMinecraftMethod = Java::Env->GetStaticMethodID(minecraftClass, "A", "()Lave;");
    if (!getMinecraftMethod) return false;

    jobject theMinecraft = Java::Env->CallStaticObjectMethod(minecraftClass, getMinecraftMethod);
    if (!theMinecraft) return false;

    jfieldID launchedVersionField = Java::Env->GetFieldID(minecraftClass, "al", "Ljava/lang/String;");
    if (!launchedVersionField) return false;

    jobject launchedVersion = Java::Env->GetObjectField(theMinecraft, launchedVersionField);
    if (!launchedVersion) return false;

    std::string version = String(launchedVersion).ToString();
    if (version == "1.8.9") { Java::Version = MinecraftVersion::VANILLA_1_8_9; return true; }

    return false;
}

static bool CheckVanilla1710()
{
    jclass minecraftClass;
    Java::AssignClass("", minecraftClass);
    if (!minecraftClass) return false;

    jmethodID getMinecraftMethod = Java::Env->GetStaticMethodID(minecraftClass, "", "");
    if (!getMinecraftMethod) return false;

    jobject theMinecraft = Java::Env->CallStaticObjectMethod(minecraftClass, getMinecraftMethod);
    if (!theMinecraft) return false;

    jfieldID launchedVersionField = Java::Env->GetFieldID(minecraftClass, "", "");
    if (!launchedVersionField) return false;

    jobject launchedVersion = Java::Env->GetObjectField(theMinecraft, launchedVersionField);
    if (!launchedVersion) return false;

    std::string version = String(launchedVersion).ToString();
    if (version == "1.7.10") { Java::Version = MinecraftVersion::VANILLA_1_7_10; return true; }

    return false;
}

static bool CheckForge189()
{
    jclass minecraftClass;
    Java::AssignClass("net.minecraft.client.Minecraft", minecraftClass);
    if (!minecraftClass) return false;

    jmethodID getMinecraftMethod = Java::Env->GetStaticMethodID(minecraftClass, "func_71410_x", "()Lnet/minecraft/client/Minecraft;");
    if (!getMinecraftMethod) return false;

    jobject theMinecraft = Java::Env->CallStaticObjectMethod(minecraftClass, getMinecraftMethod);
    if (!theMinecraft) return false;

    jfieldID launchedVersionField = Java::Env->GetFieldID(minecraftClass, "field_110447_Z", "Ljava/lang/String;");
    if (!launchedVersionField) return false;

    jobject launchedVersion = Java::Env->GetObjectField(theMinecraft, launchedVersionField);
    if (!launchedVersion) return false;

    std::string version = String(launchedVersion).ToString();
    if (version.find("1.8.9") != std::string::npos) { Java::Version = MinecraftVersion::FORGE_1_8_9; return true; }

    return false;
}

static bool CheckForge1710()
{
    jclass minecraftClass;
    Java::AssignClass("", minecraftClass);
    if (!minecraftClass) return false;

    jmethodID getMinecraftMethod = Java::Env->GetStaticMethodID(minecraftClass, "", "");
    if (!getMinecraftMethod) return false;

    jobject theMinecraft = Java::Env->CallStaticObjectMethod(minecraftClass, getMinecraftMethod);
    if (!theMinecraft) return false;

    jfieldID launchedVersionField = Java::Env->GetFieldID(minecraftClass, "", "");
    if (!launchedVersionField) return false;

    jobject launchedVersion = Java::Env->GetObjectField(theMinecraft, launchedVersionField);
    if (!launchedVersion) return false;

    std::string version = String(launchedVersion).ToString();
    if (version == "1.7.10") { Java::Version = MinecraftVersion::FORGE_1_7_10; return true; }

    return false;
}


void Java::GetMinecraftVersion()
{
    if (CheckLunarClient()) return;
    if (Java::Env->ExceptionCheck()) Java::Env->ExceptionClear();
    if (CheckVanilla189()) return;
	if (Java::Env->ExceptionCheck()) Java::Env->ExceptionClear();
	if (CheckForge189()) return;
	if (Java::Env->ExceptionCheck()) Java::Env->ExceptionClear();

	Java::Version = MinecraftVersion::UNKNOWN;
    return;

	// Skip 1.7.10 for now
	if (CheckVanilla1710()) return;
	if (CheckForge1710()) return;

}


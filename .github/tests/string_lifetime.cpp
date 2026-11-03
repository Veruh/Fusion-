#include "java/java.h"
#include "sdk/java/lang/String.h"
#include <cstring>
#include <iostream>

namespace {
char buffer[] = "Lunar 1.8.9";
bool released = false;
bool failChars = false;
bool cleared = false;
jclass JNICALL FindClass(JNIEnv*, const char*) { return reinterpret_cast<jclass>(1); }
const char* JNICALL GetChars(JNIEnv*, jstring, jboolean*) { return failChars ? nullptr : buffer; }
void JNICALL ReleaseChars(JNIEnv*, jstring, const char*) { released = true; std::memset(buffer, 'x', sizeof(buffer)-1); }
jboolean JNICALL HasException(JNIEnv*) { return failChars ? JNI_TRUE : JNI_FALSE; }
void JNICALL ClearException(JNIEnv*) { cleared = true; }
}

int main()
{
    JNINativeInterface_ table{};
    table.FindClass = FindClass;
    table.GetStringUTFChars = GetChars;
    table.ReleaseStringUTFChars = ReleaseChars;
    table.ExceptionCheck = HasException;
    table.ExceptionClear = ClearException;
    JNIEnv_ env{&table};
    Java::Env = &env;
    String text(reinterpret_cast<jobject>(1));
    if (text.ToString() != "Lunar 1.8.9" || !released) return 1;
    failChars = true;
    if (!text.ToString().empty() || !cleared) return 2;
    String nullText(static_cast<jobject>(nullptr));
    if (!nullText.ToString().empty()) return 3;
    std::cout << "JNI string lifetime and error handling passed\n";
}


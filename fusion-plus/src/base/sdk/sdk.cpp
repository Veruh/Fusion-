#include "sdk.h"
#include "util/logger.h"
#include "java/java.h"
#include "strayCache.h"

bool SDK::Init()
{
	StrayCache::Initialize();
	if (!StrayCache::initialized) return false;
	Logger::Log("Stray cache initialized");
	SDK::Minecraft = new CMinecraft();
	if (Java::Env->ExceptionCheck()) {
		Java::Env->ExceptionClear();
		return false;
	}
	if (!SDK::Minecraft->GetInstance()) return false;
	Logger::Log("Initialized Minecraft");
	return true;
}


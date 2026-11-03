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
		StrayCache::failureName = "exception creating Minecraft object";
		Java::Env->ExceptionClear();
		return false;
	}
	if (!SDK::Minecraft->GetInstance()) { StrayCache::failureName = "Minecraft instance unavailable"; return false; }
	Logger::Log("Initialized Minecraft");
	return true;
}


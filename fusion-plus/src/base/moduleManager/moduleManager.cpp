#include "moduleManager.h"

#include "modules/combat/aimAssist.h"
#include "modules/visual/esp.h"
#include "modules/movement/velocity.h"
#include "commonData.h"
#include "base.h"
#include "java/java.h"

#include <configManager/configManager.h>
#include <imgui/imgui.h>



void ModuleManager::Init()
{
	modules.push_back(std::make_unique<AimAssist>());
	modules.push_back(std::make_unique<Esp>());
	modules.push_back(std::make_unique<Velocity>());

	Logger::Log("Modules initialized");

	// Old profiles can contain invalid settings. Load them only on explicit request.
	try {
		ConfigManager::LoadFriends();
	} catch (const std::exception&) {
		settings::friends.clear();
		Logger::Log("Could not load legacy friends list");
	}
	// Start in a passive state regardless of prior sessions.
	settings::AA_Enabled = false;
	settings::ESP_Enabled = false;
	settings::Velocity_Enabled = false;
}

void ModuleManager::UpdateModules()
{
	// The update runs on one attached native thread. Discard local JNI references
	// every tick instead of retaining them until the JVM thread detaches.
	if (!Java::Env || Java::Env->PushLocalFrame(256) != JNI_OK) {
		if (Java::Env && Java::Env->ExceptionCheck()) Java::Env->ExceptionClear();
		Base::Running = false;
		return;
	}
	CommonData::UpdateData();

	for (auto& module : modules)
	{
		if (Java::Env->ExceptionCheck()) break;
		// Velocity must observe disable/focus changes to release any held key.
		module->Update();
	}
	if (Java::Env->ExceptionCheck()) {
		Java::Env->ExceptionClear();
		CommonData::dataUpdated = false;
		Base::Running = false;
	}
	CommonData::nativePlayerList.clear();
	Java::Env->PopLocalFrame(nullptr);
}

void ModuleManager::Stop()
{
	for (auto& module : modules)
		if (auto* velocity = dynamic_cast<Velocity*>(module.get())) velocity->Stop();
}

void ModuleManager::RenderUpdate()
{
	for (auto& module : modules)
	{
		if (module->IsEnabled())
			module->RenderUpdate();
	}
}

void ModuleManager::RenderMenu(int index)
{
	if (index < 0)
		return;

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);

	std::vector<std::string> categories = GetCategories();
	for (auto& module : modules)
	{
		if (module->GetCategory() == categories[index])
			module->RenderMenu();
	}
}

std::vector<std::string> ModuleManager::GetCategories()
{
	std::vector<std::string> categories;

	for (auto& module : modules)
	{
		if (std::find(categories.begin(), categories.end(), module->GetCategory()) == categories.end())
			categories.push_back(module->GetCategory());
	}

	return categories;
}


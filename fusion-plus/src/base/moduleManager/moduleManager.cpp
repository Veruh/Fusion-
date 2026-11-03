#include "moduleManager.h"

#include "modules/combat/aimAssist.h"
#include "modules/visual/esp.h"
#include "modules/movement/velocity.h"
#include "commonData.h"

#include <configManager/configManager.h>
#include <imgui/imgui.h>



void ModuleManager::Init()
{
	modules.push_back(std::make_unique<AimAssist>());
	modules.push_back(std::make_unique<Esp>());
	modules.push_back(std::make_unique<Velocity>());

	Logger::Log("Modules initialized");

	// load friends
	ConfigManager::LoadFriends();
	Logger::Log("Friends loaded");

	// load the first config
	std::vector<std::string> configList = ConfigManager::GetConfigList();
	if (!configList.empty())
	{
		ConfigManager::LoadConfig(configList[0].c_str());
		Logger::Log("Config loaded: %s", configList[0].c_str());
	}
	// Start in a passive state even when a previous profile enabled modules.
	settings::AA_Enabled = false;
	settings::ESP_Enabled = false;
	settings::Velocity_Enabled = false;
}

void ModuleManager::UpdateModules()
{
	CommonData::UpdateData();

	for (auto& module : modules)
	{
		// Velocity must observe disable/focus changes to release any held key.
		module->Update();
	}
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


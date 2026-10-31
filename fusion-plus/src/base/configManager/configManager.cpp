#include "configManager.h"
#include <util/logger.h>
#include <java/java.h>

bool ConfigManager::LoadConfig(const char* name)
{
	std::ifstream file(ConfigManager::GetConfigPath() + name + FILE_SUFFIX);

	if (!file.is_open())
		return false;

	json j;
	file >> j;
	file.close();

	if (!JsonToSettings(j))
		return false;

	return true;
}

int ConfigManager::SaveConfig(const char* name)
{
	json j;

	if (!SettingsToJson(j))
		return -1;

	std::ofstream file(ConfigManager::GetConfigPath() + name + FILE_SUFFIX);

	if (!file.is_open())
		return -1;

	file << j.dump();
	file.close();

	// Return the index of the saved config
	std::vector<std::string> configs = ConfigManager::GetConfigList();
	for (size_t i = 0; i < configs.size(); ++i)
	{
		if (configs[i] == name)
			return i;
	}

	return -1;
}

std::vector<std::string> ConfigManager::GetConfigList()
{
	std::vector<std::string> configs;

	// check if the directory exists, if not create it
	if (!std::filesystem::exists(ConfigManager::GetConfigPath()))
		std::filesystem::create_directory(ConfigManager::GetConfigPath());

	for (const auto& entry : std::filesystem::directory_iterator(ConfigManager::GetConfigPath()))
	{
		if (entry.path().extension() == FILE_SUFFIX/* || entry.path().extension() == ".json"*/)
			configs.push_back(entry.path().filename().replace_extension("").string());
	}

	return configs;
}

bool ConfigManager::SettingsToJson(json& j)
{
	// Menu
	j["Menu_AccentColor"] = settings::Menu_AccentColor;
	j["Menu_GUIMovement"] = settings::Menu_GUIMovement;
	j["Menu_Watermark"] = settings::Menu_Watermark;
	j["Menu_DisableAllRendering"] = settings::Menu_DisableAllRendering;
	j["Menu_ShowHiddenCategories"] = settings::Menu_ShowHiddenCategories;

	// ESP
	j["ESP_Enabled"] = settings::ESP_Enabled;
	j["ESP_Text"] = settings::ESP_Text;
	j["ESP_TextSize"] = settings::ESP_TextSize;
	j["ESP_TextColor"] = settings::ESP_TextColor;
	j["ESP_TextOutline"] = settings::ESP_TextOutline;
	j["ESP_TextOutlineColor"] = settings::ESP_TextOutlineColor;
	j["ESP_TextUnrenderDistance"] = settings::ESP_TextUnrenderDistance;
	j["ESP_FadeDistance"] = settings::ESP_FadeDistance;
	j["ESP_HealthBar"] = settings::ESP_HealthBar;
	j["ESP_BoxType"] = settings::ESP_BoxType;
	j["ESP_3DBoxColor"] = settings::ESP_3DBoxColor;
	j["ESP_3DBoxThickness"] = settings::ESP_3DBoxThickness;
	j["ESP_Box"] = settings::ESP_Box;
	j["ESP_BoxColor"] = settings::ESP_BoxColor;
	j["ESP_FilledBox"] = settings::ESP_FilledBox;
	j["ESP_FilledBoxColor"] = settings::ESP_FilledBoxColor;
	j["ESP_SecondFilledBoxColor"] = settings::ESP_SecondFilledBoxColor;
	j["ESP_Outline"] = settings::ESP_Outline;
	j["ESP_OutlineColor"] = settings::ESP_OutlineColor;
	j["ESP_HighlightFriends"] = settings::ESP_HighlightFriends;
	j["ESP_Friend3DBoxColor"] = settings::ESP_Friend3DBoxColor;
	j["ESP_FriendBoxColor"] = settings::ESP_FriendBoxColor;
	j["ESP_FriendFilledBoxColor"] = settings::ESP_FriendFilledBoxColor;
	j["ESP_FriendSecondFilledBoxColor"] = settings::ESP_FriendSecondFilledBoxColor;
	j["ESP_FriendOutlineColor"] = settings::ESP_FriendOutlineColor;

	// Aim Assist
	j["AA_Enabled"] = settings::AA_Enabled;
	j["AA_visibilityCheck"] = settings::AA_visibilityCheck;
	j["AA_sprintCheck"] = settings::AA_sprintCheck;
	j["AA_blockBreakCheck"] = settings::AA_blockBreakCheck;
	j["AA_weaponOnly"] = settings::AA_weaponOnly;
	j["AA_invisibleCheck"] = settings::AA_invisibleCheck;
	j["AA_mousePressCheck"] = settings::AA_mousePressCheck;
	j["AA_mouseMoveCheck"] = settings::AA_mouseMoveCheck;
	j["AA_aimAssistFeedback"] = settings::AA_aimAssistFeedback;
	j["AA_aimAssistFeedbackColor"] = settings::AA_aimAssistFeedbackColor;
	j["AA_fovCircle"] = settings::AA_fovCircle;
	j["AA_fovCircleColor"] = settings::AA_fovCircleColor;
	j["AA_adaptive"] = settings::AA_adaptive;
	j["AA_adaptiveOffset"] = settings::AA_adaptiveOffset;
	j["AA_fov"] = settings::AA_fov;
	j["AA_smooth"] = settings::AA_smooth;
	j["AA_aimDistance"] = settings::AA_aimDistance;
	j["AA_randomYaw"] = settings::AA_randomYaw;
	j["AA_randomPitch"] = settings::AA_randomPitch;
	j["AA_targetPriority"] = settings::AA_targetPriority;
	j["AA_ignoreFriends"] = settings::AA_ignoreFriends;

	// Sprint Reset
	j["SR_Enabled"] = settings::SR_Enabled;
	j["SR_DelayBetween"] = settings::SR_DelayBetween;
	j["SR_LetGoDelay"] = settings::SR_LetGoDelay;

	// Left Auto Clicker
	j["LAC_Enabled"] = settings::LAC_Enabled;
	j["LAC_leftMaxCps"] = settings::LAC_leftMaxCps;
	j["LAC_leftMinCps"] = settings::LAC_leftMinCps;
	j["LAC_ignoreBlocks"] = settings::LAC_ignoreBlocks;
	j["LAC_swordBlock"] = settings::LAC_swordBlock;
	j["LAC_swordOnly"] = settings::LAC_swordOnly;
	j["LAC_emptyHandOnly"] = settings::LAC_emptyHandOnly;

	// Right Auto Clicker
	j["RAC_Enabled"] = settings::RAC_Enabled;
	j["RAC_rightMaxCps"] = settings::RAC_rightMaxCps;
	j["RAC_rightMinCps"] = settings::RAC_rightMinCps;
	j["RAC_blocksOnly"] = settings::RAC_blocksOnly;

	// Velocity
	j["Velocity_Enabled"] = settings::Velocity_Enabled;
	j["Velocity_Mode"] = settings::Velocity_Mode;
	j["Velocity_JRReactionTime"] = settings::Velocity_JRReactionTime;
	j["Velocity_JRChange"] = settings::Velocity_JRChange;

	return true;
}

bool ConfigManager::JsonToSettings(const json& j)
{
	// Menu
	ConfigManager::GetJsonValue(j, settings::Menu_AccentColor, "Menu_AccentColor");
	ConfigManager::GetJsonValue(j, settings::Menu_GUIMovement, "Menu_GUIMovement");
	ConfigManager::GetJsonValue(j, settings::Menu_Watermark, "Menu_Watermark");
	ConfigManager::GetJsonValue(j, settings::Menu_DisableAllRendering, "Menu_DisableAllRendering");
	ConfigManager::GetJsonValue(j, settings::Menu_ShowHiddenCategories, "Menu_ShowHiddenCategories");

	// ESP
	ConfigManager::GetJsonValue(j, settings::ESP_Enabled, "ESP_Enabled");
	ConfigManager::GetJsonValue(j, settings::ESP_Text, "ESP_Text");
	ConfigManager::GetJsonValue(j, settings::ESP_TextSize, "ESP_TextSize");
	ConfigManager::GetJsonValue(j, settings::ESP_TextColor, "ESP_TextColor");
	ConfigManager::GetJsonValue(j, settings::ESP_TextOutline, "ESP_TextOutline");
	ConfigManager::GetJsonValue(j, settings::ESP_TextOutlineColor, "ESP_TextOutlineColor");
	ConfigManager::GetJsonValue(j, settings::ESP_TextUnrenderDistance, "ESP_TextUnrenderDistance");
	ConfigManager::GetJsonValue(j, settings::ESP_FadeDistance, "ESP_FadeDistance");
	ConfigManager::GetJsonValue(j, settings::ESP_HealthBar, "ESP_HealthBar");
	ConfigManager::GetJsonValue(j, settings::ESP_BoxType, "ESP_BoxType");
	ConfigManager::GetJsonValue(j, settings::ESP_3DBoxColor, "ESP_3DBoxColor");
	ConfigManager::GetJsonValue(j, settings::ESP_3DBoxThickness, "ESP_3DBoxThickness");
	ConfigManager::GetJsonValue(j, settings::ESP_Box, "ESP_Box");
	ConfigManager::GetJsonValue(j, settings::ESP_BoxColor, "ESP_BoxColor");
	ConfigManager::GetJsonValue(j, settings::ESP_FilledBox, "ESP_FilledBox");
	ConfigManager::GetJsonValue(j, settings::ESP_FilledBoxColor, "ESP_FilledBoxColor");
	ConfigManager::GetJsonValue(j, settings::ESP_SecondFilledBoxColor, "ESP_SecondFilledBoxColor");
	ConfigManager::GetJsonValue(j, settings::ESP_Outline, "ESP_Outline");
	ConfigManager::GetJsonValue(j, settings::ESP_OutlineColor, "ESP_OutlineColor");
	ConfigManager::GetJsonValue(j, settings::ESP_HighlightFriends, "ESP_HighlightFriends");
	ConfigManager::GetJsonValue(j, settings::ESP_Friend3DBoxColor, "ESP_Friend3DBoxColor");
	ConfigManager::GetJsonValue(j, settings::ESP_FriendBoxColor, "ESP_FriendBoxColor");
	ConfigManager::GetJsonValue(j, settings::ESP_FriendFilledBoxColor, "ESP_FriendFilledBoxColor");
	ConfigManager::GetJsonValue(j, settings::ESP_FriendSecondFilledBoxColor, "ESP_FriendSecondFilledBoxColor");
	ConfigManager::GetJsonValue(j, settings::ESP_FriendOutlineColor, "ESP_FriendOutlineColor");

	// Aim Assist
	ConfigManager::GetJsonValue(j, settings::AA_Enabled, "AA_Enabled");
	ConfigManager::GetJsonValue(j, settings::AA_visibilityCheck, "AA_visibilityCheck");
	ConfigManager::GetJsonValue(j, settings::AA_sprintCheck, "AA_sprintCheck");
	ConfigManager::GetJsonValue(j, settings::AA_blockBreakCheck, "AA_blockBreakCheck");
	ConfigManager::GetJsonValue(j, settings::AA_weaponOnly, "AA_weaponOnly");
	ConfigManager::GetJsonValue(j, settings::AA_invisibleCheck, "AA_invisibleCheck");
	ConfigManager::GetJsonValue(j, settings::AA_mousePressCheck, "AA_mousePressCheck");
	ConfigManager::GetJsonValue(j, settings::AA_mouseMoveCheck, "AA_mouseMoveCheck");
	ConfigManager::GetJsonValue(j, settings::AA_aimAssistFeedback, "AA_aimAssistFeedback");
	ConfigManager::GetJsonValue(j, settings::AA_aimAssistFeedbackColor, "AA_aimAssistFeedbackColor");
	ConfigManager::GetJsonValue(j, settings::AA_fovCircle, "AA_fovCircle");
	ConfigManager::GetJsonValue(j, settings::AA_fovCircleColor, "AA_fovCircleColor");
	ConfigManager::GetJsonValue(j, settings::AA_adaptive, "AA_adaptive");
	ConfigManager::GetJsonValue(j, settings::AA_adaptiveOffset, "AA_adaptiveOffset");
	ConfigManager::GetJsonValue(j, settings::AA_fov, "AA_fov");
	ConfigManager::GetJsonValue(j, settings::AA_smooth, "AA_smooth");
	ConfigManager::GetJsonValue(j, settings::AA_aimDistance, "AA_aimDistance");
	ConfigManager::GetJsonValue(j, settings::AA_randomYaw, "AA_randomYaw");
	ConfigManager::GetJsonValue(j, settings::AA_randomPitch, "AA_randomPitch");
	ConfigManager::GetJsonValue(j, settings::AA_targetPriority, "AA_targetPriority");
	ConfigManager::GetJsonValue(j, settings::AA_ignoreFriends, "AA_ignoreFriends");

	// Sprint Reset
	ConfigManager::GetJsonValue(j, settings::SR_Enabled, "SR_Enabled");
	ConfigManager::GetJsonValue(j, settings::SR_DelayBetween, "SR_DelayBetween");
	ConfigManager::GetJsonValue(j, settings::SR_LetGoDelay, "SR_LetGoDelay");

	// Left Auto Clicker
	ConfigManager::GetJsonValue(j, settings::LAC_Enabled, "LAC_Enabled");
	ConfigManager::GetJsonValue(j, settings::LAC_leftMaxCps, "LAC_leftMaxCps");
	ConfigManager::GetJsonValue(j, settings::LAC_leftMinCps, "LAC_leftMinCps");
	ConfigManager::GetJsonValue(j, settings::LAC_ignoreBlocks, "LAC_ignoreBlocks");
	ConfigManager::GetJsonValue(j, settings::LAC_swordBlock, "LAC_swordBlock");
	ConfigManager::GetJsonValue(j, settings::LAC_swordOnly, "LAC_swordOnly");
	ConfigManager::GetJsonValue(j, settings::LAC_emptyHandOnly, "LAC_emptyHandOnly");

	// Right Auto Clicker
	ConfigManager::GetJsonValue(j, settings::RAC_Enabled, "RAC_Enabled");
	ConfigManager::GetJsonValue(j, settings::RAC_rightMaxCps, "RAC_rightMaxCps");
	ConfigManager::GetJsonValue(j, settings::RAC_rightMinCps, "RAC_rightMinCps");
	ConfigManager::GetJsonValue(j, settings::RAC_blocksOnly, "RAC_blocksOnly");

	// Velocity
	ConfigManager::GetJsonValue(j, settings::Velocity_Enabled, "Velocity_Enabled");
	ConfigManager::GetJsonValue(j, settings::Velocity_Mode, "Velocity_Mode");
	ConfigManager::GetJsonValue(j, settings::Velocity_JRReactionTime, "Velocity_JRReactionTime");
	ConfigManager::GetJsonValue(j, settings::Velocity_JRChange, "Velocity_JRChange");
	
	return true;
}

std::string ConfigManager::GetDocumentsPath()
{
	char path[MAX_PATH];

	// Get the path to the Documents folder (CSIDL_PERSONAL refers to "My Documents")
	if (SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, path) == S_OK) {
		return std::string(path);
	}
	else {
		std::cerr << "Failed to get Documents folder." << std::endl;
		return "";
	}
}

std::string ConfigManager::GetConfigPath()
{
	std::string path = ConfigManager::GetFusionPath() + "/configs/";

	if (!std::filesystem::exists(path))
		std::filesystem::create_directories(path);

	return path;
}

std::string ConfigManager::GetFusionPath()
{
	std::string path = ConfigManager::GetDocumentsPath() + "/Fusion+/";

	if (!std::filesystem::exists(path))
		std::filesystem::create_directories(path);

	return path;
}

bool ConfigManager::LoadFriends()
{
	std::string filePath = ConfigManager::GetFusionPath() + "friends.json";

	if (!std::filesystem::exists(filePath))
		return false;

	std::ifstream file(filePath);
	if (!file.is_open())
		return false;

	json j;
	file >> j;
	file.close();

	if (!j.is_array())
		return false;

	settings::friends.clear();
	for (const auto& friendName : j)
	{
		settings::friends.push_back(friendName.get<std::string>());
	}

	return true;
}

bool ConfigManager::SaveFriends()
{
	std::string filePath = ConfigManager::GetFusionPath() + "friends.json";

	// Ensure the directory exists before writing
	std::string dirPath = ConfigManager::GetFusionPath();
	if (!std::filesystem::exists(dirPath))
		std::filesystem::create_directories(dirPath);

	json j;
	for (const auto& friendName : settings::friends)
	{
		j.push_back(friendName);
	}

	std::ofstream file(filePath);
	if (!file.is_open())
		return false;

	file << j.dump(4);
	file.close();

	return true;
}


bool ConfigManager::AddFriend(const std::string& name)
{
	if (IsFriend(name))
		return false;

	settings::friends.push_back(name);

	return SaveFriends();
}

bool ConfigManager::RemoveFriend(const std::string& name)
{
	auto it = std::find(settings::friends.begin(), settings::friends.end(), name);

	if (it == settings::friends.end())
		return false;

	settings::friends.erase(it);

	return SaveFriends();
}

bool ConfigManager::IsFriend(const std::string& name)
{
	return std::find(settings::friends.begin(), settings::friends.end(), name) != settings::friends.end();
}

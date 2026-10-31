#pragma once

#include <string>
#include <vector>

struct settings {
	inline static std::vector<std::string> friends;

	// Menu
	inline static float Menu_AccentColor[4]{ 0, 0.4f, 0.4f, 1.0f };
	inline static bool Menu_GUIMovement = false;
	inline static bool Menu_Watermark = true;
	inline static bool Menu_DisableAllRendering = false;
	inline static bool Menu_ShowHiddenCategories = false;
	inline static std::vector<std::string> Menu_HiddenCategoriesList{ };

	// ESP
	inline static bool ESP_Enabled = false;
	inline static bool ESP_Text = true;
	inline static float ESP_TextSize = 18;
	inline static float ESP_TextColor[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
	inline static bool ESP_TextOutline = true;
	inline static float ESP_TextOutlineColor[4]{ 0, 0, 0, 1.0f };
	inline static float ESP_TextUnrenderDistance = 14.0f;
	inline static float ESP_FadeDistance = 3.0f;
	inline static bool ESP_HealthBar = true;
	inline static int ESP_BoxType = 0;
	inline static const char* ESP_BoxTypeList[4]{ "Dynamic 2D Box", "3D Box" };
	inline static float ESP_3DBoxColor[4]{ 0, 0, 0, 1 };
	inline static float ESP_3DBoxThickness = 1.0f;
	inline static bool ESP_Box = true;
	inline static float ESP_BoxColor[4]{ 0, 0, 0, 1 };
	inline static bool ESP_FilledBox = true;
	inline static float ESP_FilledBoxColor[4]{ 0, 0, 0, 0.15f };
	inline static float ESP_SecondFilledBoxColor[4]{ 0, 0, 0, 0.15f };
	inline static bool ESP_Outline = true;
	inline static float ESP_OutlineColor[4]{ 0, 0, 0, 0.25 };
	inline static bool ESP_HighlightFriends = true;
	inline static float ESP_Friend3DBoxColor[4]{ 0, 1, 0, 1 };
	inline static float ESP_FriendBoxColor[4]{ 0, 1, 0, 1 };
	inline static float ESP_FriendFilledBoxColor[4]{ 0, 1, 0, 0.15f };
	inline static float ESP_FriendSecondFilledBoxColor[4]{ 0, 1, 0, 0.15f };
	inline static float ESP_FriendOutlineColor[4]{ 0, 1, 0, 0.25 };

	// Aim Assist
	inline static bool AA_Enabled = false;
	inline static bool AA_visibilityCheck = true;
	inline static bool AA_sprintCheck = false;
	inline static bool AA_blockBreakCheck = true;
	inline static bool AA_weaponOnly = true;
	inline static bool AA_invisibleCheck = true;
	inline static bool AA_mousePressCheck = true;
	inline static bool AA_mouseMoveCheck = false;
	inline static bool AA_aimAssistFeedback = true;
	inline static float AA_aimAssistFeedbackColor[4]{ 1.f, 1.f, 1.f, 1.f };
	inline static bool AA_fovCircle = true;
	inline static float AA_fovCircleColor[4]{ 1.f, 1.f, 1.f, 1.f };
	inline static bool AA_adaptive = true;
	inline static float AA_adaptiveOffset = 3.f;
	inline static float AA_fov = 35.0f;
	inline static float AA_smooth = 15.f;
	inline static float AA_aimDistance = 4.f;
	inline static float AA_randomYaw = 2;
	inline static float AA_randomPitch = .075f;
	inline static int AA_targetPriority = 2;
	inline static const char* AA_targetPriorityList[3]{ "Distance", "Health", "Closest to Crosshair" };
	inline static bool AA_ignoreFriends = true;

	// Sprint Reset
	inline static bool SR_Enabled = false;
	inline static float SR_DelayBetween = .5f;
	inline static float SR_LetGoDelay = .05f;

	// Left Auto Clicker
	inline static bool LAC_Enabled = false;
	inline static float LAC_leftMaxCps = 18;
	inline static float LAC_leftMinCps = 13;
	inline static bool LAC_ignoreBlocks = true;
	inline static bool LAC_swordBlock = true;
	inline static bool LAC_swordOnly = false;
	inline static bool LAC_emptyHandOnly = false;

	// Right Auto Clicker
	inline static bool RAC_Enabled = false;
	inline static float RAC_rightMaxCps = 15;
	inline static float RAC_rightMinCps = 12;
	inline static bool RAC_blocksOnly = true;

	// Velocity
	inline static bool Velocity_Enabled = false;
	inline static int Velocity_Mode = 0;
	inline static const char* Velocity_ModeList[0]{ "Jump Reset" };
	inline static float Velocity_JRReactionTime = 0.1f;
	inline static int Velocity_JRChange = 80;

};

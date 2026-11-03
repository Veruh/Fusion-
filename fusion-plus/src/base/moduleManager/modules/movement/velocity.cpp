#include "velocity.h"

#include <imgui/imgui.h>
#include "menu/menu.h"
#include <moduleManager/commonData.h>

inline static bool send_key(WORD vk_key, bool send_down = true)
{
	static INPUT ip{ INPUT_KEYBOARD };

	ip.ki.wScan = 0;
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;
	ip.ki.wVk = vk_key;
	ip.ki.dwFlags = send_down ? 0 : KEYEVENTF_KEYUP;

	return SendInput(1, &ip, sizeof(INPUT)) == 1;
}

void Velocity::Stop()
{
	if (syntheticJumpHeld && send_key(VK_SPACE, false)) syntheticJumpHeld = false;
}

void Velocity::Update()
{
	const bool gameFocused = Menu::HandleWindow && GetForegroundWindow() == Menu::HandleWindow;
	if (!settings::Velocity_Enabled || !gameFocused || Menu::Open || !CommonData::SanityCheck() || SDK::Minecraft->IsInGuiState())
	{
		Stop();
		canBeHit = true;
		return;
	}
	if (syntheticJumpHeld && std::chrono::steady_clock::now() >= releaseAt) Stop();

	CEntityPlayerSP* player = SDK::Minecraft->thePlayer;

	int hurtResistantTime = player->GetHurtResistantTime();

	if (!canBeHit && hurtResistantTime == 0)
	{
		canBeHit = true;
	}

	if (!canBeHit || syntheticJumpHeld)
	{
		return;
	}

	if (hurtResistantTime <= 10)
	{
		return;
	}

	if (settings::Velocity_Mode == 0)
	{
		const int chance = (std::clamp)(settings::Velocity_JRChange, 0, 100);
		if (std::uniform_int_distribution<int>(1, 100)(random) <= chance && !(GetAsyncKeyState(VK_SPACE) & 0x8000))
		{
			syntheticJumpHeld = send_key(VK_SPACE);
			if (syntheticJumpHeld) {
				releaseAt = std::chrono::steady_clock::now() + std::chrono::milliseconds(std::uniform_int_distribution<int>(200, 350)(random));
				canBeHit = false;
			}
		}
	}
}

void Velocity::RenderMenu()
{
	static bool renderSettings = false;

	ImGui::BeginGroup();

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.12f, 0.5));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10);

	if (ImGui::BeginChild("velocity_header", ImVec2(425.f, renderSettings ? 128.f : 35.f), false))
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
		ImGui::BeginGroup();
		Menu::ToggleButton(68, ("Toggle " + this->GetName()).c_str(), ImVec2(368, 0), &settings::Velocity_Enabled);
		ImGui::EndGroup();
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			renderSettings = !renderSettings;
		}

		ImGui::PopStyleColor();
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.12f, 0.0));

		if (renderSettings)
		{
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
			ImGui::Separator();
			if (ImGui::BeginChild("velocity_settings", ImVec2(425, 83), false))
			{
				Menu::ComboBox(69, "Mode", ImVec2(270, 0), &settings::Velocity_Mode, settings::Velocity_ModeList, 1);

				if (settings::Velocity_Mode == 0)
				{
					Menu::Slider(70, "Reaction Time (ms)", ImVec2(225, 0), &settings::Velocity_JRReactionTime, 0, 1000);
					Menu::Slider(71, "Chance %", ImVec2(235, 0), &settings::Velocity_JRChange, 0, 100);
				}
			}
			ImGui::EndChild();
			ImGui::Spacing();
		}
	}
	ImGui::EndChild();

	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	ImGui::EndGroup();
}


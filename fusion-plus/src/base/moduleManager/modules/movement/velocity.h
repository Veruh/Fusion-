#pragma once

#include <string>

#include "moduleManager/moduleBase.h"

#include "util/math/geometry.h"

#include <random>
#include <chrono>

class Velocity : public ModuleBase
{
public:
	void Update() override;
	void Stop();
	void RenderUpdate() override {}

	void RenderMenu() override;

	bool IsEnabled() override { return settings::Velocity_Enabled; }
	std::string GetName() override { return Name; }
	std::string GetCategory() override { return Category; }

private:
	std::string Name = "Velocity";
	std::string Category = "Movement";
	std::mt19937 random{std::random_device{}()};
	std::chrono::steady_clock::time_point releaseAt{};
	bool syntheticJumpHeld = false;
	bool canBeHit = true;
};


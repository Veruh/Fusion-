#pragma once
#include <mutex>

struct Base
{
	static void Init();
	static void Kill();

	static void CheatLoop();
	static void RenderLoop();

	static inline bool Running;
	// Guards shared settings, module snapshots, ImGui state and window state.
	static inline std::recursive_mutex StateMutex;
};



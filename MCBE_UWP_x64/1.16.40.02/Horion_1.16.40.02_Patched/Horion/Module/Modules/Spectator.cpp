#include "Spectator.h"

#include <Windows.h>

#include "../../../Utils/Logger.h"
#include "../../../Utils/Utils.h"

static uint8_t* g_BlockOverlayFunc = nullptr;
static uint8_t g_BlockOverlayOriginal[3]{};
static bool g_BlockOverlayPatched = false;

static void tryPatchBlockOverlay(bool disableOverlay) {
	if (g_BlockOverlayFunc == nullptr) {
		g_BlockOverlayFunc = reinterpret_cast<uint8_t*>(
			FindSignature("48 8B C4 41 56 48 81 EC A0 00 00 00"));

		if (g_BlockOverlayFunc != nullptr) {
			memcpy(g_BlockOverlayOriginal, g_BlockOverlayFunc, sizeof(g_BlockOverlayOriginal));
		}
	}

	if (g_BlockOverlayFunc == nullptr)
		return;

	DWORD oldProtect = 0;
	if (!VirtualProtect(g_BlockOverlayFunc, 3, PAGE_EXECUTE_READWRITE, &oldProtect)) {
#ifdef _DEBUG
		logF("Spectator: VirtualProtect failed for block overlay patch");
		__debugbreak();
#endif
		return;
	}

	if (disableOverlay) {
		// 48 8B C4  -> B0 00 C3
		g_BlockOverlayFunc[0] = 0xB0;
		g_BlockOverlayFunc[1] = 0x00;
		g_BlockOverlayFunc[2] = 0xC3;
		g_BlockOverlayPatched = true;
	} else if (g_BlockOverlayPatched) {
		memcpy(g_BlockOverlayFunc, g_BlockOverlayOriginal, sizeof(g_BlockOverlayOriginal));
		g_BlockOverlayPatched = false;
	}

	VirtualProtect(g_BlockOverlayFunc, 3, oldProtect, &oldProtect);
}

Spectator::Spectator() : IModule(0, Category::MOVEMENT, "Spectator-like: phase + disable block overlay") {
}

Spectator::~Spectator() {
}

const char* Spectator::getModuleName() {
	return "Spectator";
}

void Spectator::onEnable() {
	tryPatchBlockOverlay(true);
}

void Spectator::onTick(C_GameMode* gm) {
	// Phase behavior (from old Phase module)
	gm->player->aabb.upper.y = gm->player->aabb.lower.y;
}

void Spectator::onDisable() {
	tryPatchBlockOverlay(false);

	if (g_Data.getLocalPlayer() != nullptr)
		g_Data.getLocalPlayer()->aabb.upper.y += 1.8f;
}


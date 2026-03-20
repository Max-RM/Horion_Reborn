#include "AutoCordsTest.h"

#include <algorithm>
#include <vector>
#include <string>

#include "../../Command/CommandMgr.h"
#include "../../DrawUtils.h"
#include "../../../Utils/Utils.h"

static const char* s_modeNames[] = {
	"SETBLOCK",
	"FILL",
	"CLONE",
	"TICKINGAREA",
	"TESTFORBLOCKS"
};

AutoCordsTest::AutoCordsTest()
	: IModule(0, Category::WORLD, "Auto coordinates helper for command blocks") {
	registerBoolSetting("SetBlock", &modeSetBlock, modeSetBlock);
	registerBoolSetting("Fill", &modeFill, modeFill);
	registerBoolSetting("Clone", &modeClone, modeClone);

	// Indented clone submodes (must be right after Clone)
	registerBoolSetting("~ filtered", &cloneMaskFiltered, cloneMaskFiltered);
	registerBoolSetting("~ masked", &cloneMaskMasked, cloneMaskMasked);
	registerBoolSetting("~ replace", &cloneMaskReplace, cloneMaskReplace);

	registerBoolSetting("  ~ force", &cloneMoveForce, cloneMoveForce);
	registerBoolSetting("  ~ move", &cloneMoveMove, cloneMoveMove);
	registerBoolSetting("  ~ normal", &cloneMoveNormal, cloneMoveNormal);

	registerBoolSetting("TickingArea", &modeTickingArea, modeTickingArea);
	registerBoolSetting("TestForBlocks", &modeTestForBlocks, modeTestForBlocks);

	// Visual separator between mode group and behavior group
    //registerBoolSetting("--------------------------------------------------", &separatorModes, separatorModes);
	
	registerBoolSetting("TP back", &teleportBackToOrigin, teleportBackToOrigin);
	registerBoolSetting("Axe tool mode", &axeToolMode, axeToolMode);
	registerBoolSetting("Give block item", &giveBlockItem, giveBlockItem);
	registerBoolSetting("Render selection", &renderSelection, renderSelection);
	registerFloatSetting("Render alpha", &renderAlpha, renderAlpha, 0.05f, 1.0f);

	registerBoolSetting("Struct LOAD", &structLoad, structLoad);
	registerBoolSetting("Struct SAVE", &structSave, structSave);
}

void AutoCordsTest::onLevelRender() {
	if (!renderSelection)
		return;
	if (g_Data.getLocalPlayer() == nullptr)
		return;
	// Don't render while ClickGUI is open (keeps it clean)
	auto clickGui = moduleMgr->getModule<ClickGuiMod>();
	if (clickGui != nullptr && clickGui->isEnabled())
		return;

	// Draw boxes around saved points
	for (int i = 0; i < clickCount && i < 4; i++) {
		const vec3_ti& b = clickedBlocks[i];
		if (b == vec3_ti(0, 0, 0))
			continue;
		vec3_t lower = b.toFloatVector();
		vec3_t upper = lower.add(vec3_t(1.f, 1.f, 1.f));
		DrawUtils::setColor(0.2f, 0.8f, 1.0f, renderAlpha);
		DrawUtils::drawBox3d(lower, upper);
	}

	// If we have a pair that defines a region (fill/save), draw the region bounds too
	if (clickCount >= 3) {
		vec3_ti a = clickedBlocks[1];
		vec3_ti b = clickedBlocks[2];
		if (a != vec3_ti(0, 0, 0) && b != vec3_ti(0, 0, 0)) {
			vec3_t lower((float)std::min(a.x, b.x), (float)std::min(a.y, b.y), (float)std::min(a.z, b.z));
			vec3_t upper((float)std::max(a.x, b.x) + 1.f, (float)std::max(a.y, b.y) + 1.f, (float)std::max(a.z, b.z) + 1.f);
			DrawUtils::setColor(1.0f, 0.6f, 0.2f, renderAlpha);
			DrawUtils::drawBox3d(lower, upper);
		}
	}
}

AutoCordsTest::~AutoCordsTest() {
}

const char* AutoCordsTest::getModuleName() {
	return "AutoCords";
}

int AutoCordsTest::getRequiredClicks() const {
	if (originIsStructureBlock) {
		if (structLoad)
			return 2;  // origin + target
		if (structSave)
			return 3;  // origin + A + B
		// If structure origin but no struct mode selected, fall back to 2 (acts like setblock)
		return 2;
	}

	switch (currentMode) {
	case Mode::SetBlock:     return 2;
	case Mode::Fill:         return 3;
	case Mode::Clone:        return 4;
	case Mode::TickingArea:  return 3;
	case Mode::TestForBlocks:return 4;
	}
	return 2;
}

void AutoCordsTest::resetSelection() {
	clickCount = 0;
	originBlock = vec3_ti(0, 0, 0);
	for (auto& b : clickedBlocks)
		b = vec3_ti(0, 0, 0);
	hasSavedPlayerState = false;
	originIsStructureBlock = false;
}

void AutoCordsTest::syncStructureMode() {
	// Struct LOAD/SAVE are mutually exclusive, but allow both-off.
	auto wasTurnedOn = [](bool now, bool prev) { return now && !prev; };
	bool loadOn = wasTurnedOn(structLoad, prevStructLoad);
	bool saveOn = wasTurnedOn(structSave, prevStructSave);

	if (loadOn && saveOn) {
		// Last writer wins; keep LOAD
		structSave = false;
	} else if (loadOn) {
		structSave = false;
	} else if (saveOn) {
		structLoad = false;
	}

	prevStructLoad = structLoad;
	prevStructSave = structSave;
}

std::string AutoCordsTest::buildStructureSnbtLoad(const vec3_ti& origin, const vec3_ti& target) const {
	int dx = target.x - origin.x;
	int dy = target.y - origin.y;
	int dz = target.z - origin.z;
	return "{Count:1b,Damage:0s,Name:\"minecraft:structure_block\",WasPickedUp:0b,tag:{data:1,dataField:\"\",display:{Lore:[\"(+DATA)\"]},ignoreEntities:0b,includePlayers:0b,integrity:100f,isPowered:0b,mirror:0b,redstoneSaveMode:0,removeBlocks:0b,rotation:0b,seed:0l,showBoundingBox:1b,structureName:\"\",xStructureOffset:"
		+ std::to_string(dx) + ",xStructureSize:1,yStructureOffset:" + std::to_string(dy) + ",yStructureSize:1,zStructureOffset:" + std::to_string(dz) + ",zStructureSize:1}}";
}

std::string AutoCordsTest::buildStructureSnbtSave(const vec3_ti& origin, const vec3_ti& a, const vec3_ti& b) const {
	int minX = std::min(a.x, b.x);
	int minY = std::min(a.y, b.y);
	int minZ = std::min(a.z, b.z);
	int maxX = std::max(a.x, b.x);
	int maxY = std::max(a.y, b.y);
	int maxZ = std::max(a.z, b.z);

	int offX = minX - origin.x;
	int offY = minY - origin.y;
	int offZ = minZ - origin.z;
	int sizeX = (maxX - minX) + 1;
	int sizeY = (maxY - minY) + 1;
	int sizeZ = (maxZ - minZ) + 1;

	return "{Count:1b,Damage:0s,Name:\"minecraft:structure_block\",WasPickedUp:0b,tag:{data:1,dataField:\"\",display:{Lore:[\"(+DATA)\"]},ignoreEntities:0b,includePlayers:0b,integrity:100f,isPowered:0b,mirror:0b,redstoneSaveMode:0,removeBlocks:0b,rotation:0b,seed:0l,showBoundingBox:1b,structureName:\"\",xStructureOffset:"
		+ std::to_string(offX) + ",xStructureSize:" + std::to_string(sizeX)
		+ ",yStructureOffset:" + std::to_string(offY) + ",yStructureSize:" + std::to_string(sizeY)
		+ ",zStructureOffset:" + std::to_string(offZ) + ",zStructureSize:" + std::to_string(sizeZ) + "}}";
}

void AutoCordsTest::syncModeFromSettings() {
	auto wasTurnedOn = [](bool now, bool prev) { return now && !prev; };

	Mode selected = currentMode;
	bool found = false;

	if (wasTurnedOn(modeSetBlock, prevModeSetBlock)) { selected = Mode::SetBlock; found = true; }
	else if (wasTurnedOn(modeFill, prevModeFill)) { selected = Mode::Fill; found = true; }
	else if (wasTurnedOn(modeClone, prevModeClone)) { selected = Mode::Clone; found = true; }
	else if (wasTurnedOn(modeTickingArea, prevModeTickingArea)) { selected = Mode::TickingArea; found = true; }
	else if (wasTurnedOn(modeTestForBlocks, prevModeTestForBlocks)) { selected = Mode::TestForBlocks; found = true; }

	if (!found) {
		if (modeFill) selected = Mode::Fill;
		else if (modeClone) selected = Mode::Clone;
		else if (modeTickingArea) selected = Mode::TickingArea;
		else if (modeTestForBlocks) selected = Mode::TestForBlocks;
		else selected = Mode::SetBlock;
	}

	modeSetBlock = (selected == Mode::SetBlock);
	modeFill = (selected == Mode::Fill);
	modeClone = (selected == Mode::Clone);
	modeTickingArea = (selected == Mode::TickingArea);
	modeTestForBlocks = (selected == Mode::TestForBlocks);

	prevModeSetBlock = modeSetBlock;
	prevModeFill = modeFill;
	prevModeClone = modeClone;
	prevModeTickingArea = modeTickingArea;
	prevModeTestForBlocks = modeTestForBlocks;

	if (selected != currentMode) {
		currentMode = selected;
		resetSelection();
	}

	// Keep clone submodes consistent when switching away from clone
	if (currentMode != Mode::Clone) {
		cloneMaskFiltered = cloneMaskMasked = cloneMaskReplace = false;
		cloneMoveForce = cloneMoveMove = cloneMoveNormal = false;
		prevCloneMaskFiltered = prevCloneMaskMasked = prevCloneMaskReplace = false;
		prevCloneMoveForce = prevCloneMoveMove = prevCloneMoveNormal = false;
	}
}

void AutoCordsTest::syncCloneSubmodes() {
	if (currentMode != Mode::Clone) {
		return;
	}

	auto wasTurnedOn = [](bool now, bool prev) { return now && !prev; };

	// Mask modes
	enum class MaskSel { None, Filtered, Masked, Replace };
	MaskSel maskSel = MaskSel::None;

	if (wasTurnedOn(cloneMaskFiltered, prevCloneMaskFiltered)) maskSel = MaskSel::Filtered;
	else if (wasTurnedOn(cloneMaskMasked, prevCloneMaskMasked)) maskSel = MaskSel::Masked;
	else if (wasTurnedOn(cloneMaskReplace, prevCloneMaskReplace)) maskSel = MaskSel::Replace;
	else {
		if (cloneMaskFiltered) maskSel = MaskSel::Filtered;
		else if (cloneMaskMasked) maskSel = MaskSel::Masked;
		else if (cloneMaskReplace) maskSel = MaskSel::Replace;
	}

	if (maskSel == MaskSel::None) {
		cloneMaskFiltered = cloneMaskMasked = cloneMaskReplace = false;
		cloneMoveForce = cloneMoveMove = cloneMoveNormal = false;
	} else {
		cloneMaskFiltered = (maskSel == MaskSel::Filtered);
		cloneMaskMasked = (maskSel == MaskSel::Masked);
		cloneMaskReplace = (maskSel == MaskSel::Replace);
	}

	// Move modes only valid when a mask is active
	bool anyMask = (maskSel != MaskSel::None);
	if (!anyMask) {
		cloneMoveForce = cloneMoveMove = cloneMoveNormal = false;
	} else {
		enum class MoveSel { None, Force, Move, Normal };
		MoveSel moveSel = MoveSel::None;

		if (wasTurnedOn(cloneMoveForce, prevCloneMoveForce)) moveSel = MoveSel::Force;
		else if (wasTurnedOn(cloneMoveMove, prevCloneMoveMove)) moveSel = MoveSel::Move;
		else if (wasTurnedOn(cloneMoveNormal, prevCloneMoveNormal)) moveSel = MoveSel::Normal;
		else {
			if (cloneMoveForce) moveSel = MoveSel::Force;
			else if (cloneMoveMove) moveSel = MoveSel::Move;
			else if (cloneMoveNormal) moveSel = MoveSel::Normal;
		}

		cloneMoveForce = (moveSel == MoveSel::Force);
		cloneMoveMove = (moveSel == MoveSel::Move);
		cloneMoveNormal = (moveSel == MoveSel::Normal);
	}

	prevCloneMaskFiltered = cloneMaskFiltered;
	prevCloneMaskMasked = cloneMaskMasked;
	prevCloneMaskReplace = cloneMaskReplace;
	prevCloneMoveForce = cloneMoveForce;
	prevCloneMoveMove = cloneMoveMove;
	prevCloneMoveNormal = cloneMoveNormal;
}

bool AutoCordsTest::isHoldingWoodenAxe() const {
	auto lp = g_Data.getLocalPlayer();
	if (lp == nullptr)
		return false;
	auto supplies = lp->getSupplies();
	if (supplies == nullptr || supplies->inventory == nullptr)
		return false;
	auto stack = supplies->inventory->getItemStack(supplies->selectedHotbarSlot);
	return stack != nullptr && stack->item != nullptr && *stack->item != nullptr && (*stack->item)->itemId == 271;
}

std::string AutoCordsTest::buildCommand(const vec3_ti* clicks, int count) const {
	const vec3_ti& origin = clicks[0];
	std::vector<std::string> rel;
	for (int i = 1; i < count; i++) {
		vec3_ti p = clicks[i];
		int dx = p.x - origin.x;
		int dy = p.y - origin.y;
		int dz = p.z - origin.z;

		auto tilde = [](int d) -> std::string {
			if (d == 0) return "~";
			return "~" + std::to_string(d);
		};

		rel.push_back(tilde(dx));
		rel.push_back(tilde(dy));
		rel.push_back(tilde(dz));
	}

	std::string joined;
	for (size_t i = 0; i < rel.size(); i++) {
		if (i > 0) joined.push_back(' ');
		joined += rel[i];
	}

	switch (currentMode) {
	case Mode::SetBlock:
		return "/setblock " + joined + (setblockBlockText.empty() ? "" : (" " + setblockBlockText));
	case Mode::Fill:
		return "/fill " + joined + (fillBlockText.empty() ? "" : (" " + fillBlockText));
	case Mode::Clone: {
		std::string cmd = "/clone " + joined;
		std::string maskMode;
		std::string moveMode;

		if (cloneMaskFiltered) maskMode = "filtered";
		else if (cloneMaskMasked) maskMode = "masked";
		else if (cloneMaskReplace) maskMode = "replace";

		if (!maskMode.empty()) {
			if (cloneMoveForce) moveMode = "force";
			else if (cloneMoveMove) moveMode = "move";
			else if (cloneMoveNormal) moveMode = "normal";
		}

		if (!maskMode.empty())
			cmd += " " + maskMode;
		if (!moveMode.empty())
			cmd += " " + moveMode;
		return cmd;
	}
	case Mode::TickingArea:
		return "/tickingarea add " + joined;
	case Mode::TestForBlocks:
		return "/testforblocks " + joined;
	}
	return joined;
}

void AutoCordsTest::setModeFromConsole(Mode m) {
	// Toggle the mutually exclusive command mode checkboxes
	modeSetBlock = (m == Mode::SetBlock);
	modeFill = (m == Mode::Fill);
	modeClone = (m == Mode::Clone);
	modeTickingArea = (m == Mode::TickingArea);
	modeTestForBlocks = (m == Mode::TestForBlocks);
	// Make prev match so UI doesn't "bounce"
	prevModeSetBlock = modeSetBlock;
	prevModeFill = modeFill;
	prevModeClone = modeClone;
	prevModeTickingArea = modeTickingArea;
	prevModeTestForBlocks = modeTestForBlocks;
	currentMode = m;
	resetSelection();
}

void AutoCordsTest::setCloneMaskFromConsole(const std::string& mask) {
	// all-off allowed
	cloneMaskFiltered = cloneMaskMasked = cloneMaskReplace = false;
	if (mask == "filtered") cloneMaskFiltered = true;
	else if (mask == "masked") cloneMaskMasked = true;
	else if (mask == "replace") cloneMaskReplace = true;
	prevCloneMaskFiltered = cloneMaskFiltered;
	prevCloneMaskMasked = cloneMaskMasked;
	prevCloneMaskReplace = cloneMaskReplace;

	// if mask cleared -> also clear move
	if (!(cloneMaskFiltered || cloneMaskMasked || cloneMaskReplace)) {
		cloneMoveForce = cloneMoveMove = cloneMoveNormal = false;
		prevCloneMoveForce = prevCloneMoveMove = prevCloneMoveNormal = false;
	}
}

void AutoCordsTest::setCloneMoveFromConsole(const std::string& move) {
	cloneMoveForce = cloneMoveMove = cloneMoveNormal = false;
	if (move == "force") cloneMoveForce = true;
	else if (move == "move") cloneMoveMove = true;
	else if (move == "normal") cloneMoveNormal = true;
	prevCloneMoveForce = cloneMoveForce;
	prevCloneMoveMove = cloneMoveMove;
	prevCloneMoveNormal = cloneMoveNormal;
}

void AutoCordsTest::setStructModeFromConsole(const std::string& mode) {
	if (mode == "load") {
		structLoad = true;
		structSave = false;
	} else if (mode == "save") {
		structLoad = false;
		structSave = true;
	} else {
		structLoad = false;
		structSave = false;
	}
	prevStructLoad = structLoad;
	prevStructSave = structSave;
	resetSelection();
}

void AutoCordsTest::onEnable() {
	syncModeFromSettings();
	resetSelection();
	hasClicked = false;

	g_Data.getGuiData()->displayClientMessageF(
		"%s[AutoCords]%s is a tool to simplify working with "
		"relative coordinates for command blocks, or creating structure blocks. "
		"First click must be on a command or structure block, then click on other points. "
		"Clicks are counted per mode (%s). Restart the module to reset if you mis-click.",
		YELLOW, GRAY, s_modeNames[(int)currentMode]);

	if (giveBlockItem) {
		g_Data.getGuiData()->displayClientMessageF(
			"%s[AutoCords]%s Give block item is ON: after finishing selection, "
			"your currently selected hotbar item will be overwritten using %cnbt load.",
			YELLOW, GRAY, cmdMgr->prefix);
	}
}

void AutoCordsTest::onDisable() {
	resetSelection();
	hasClicked = false;
}

void AutoCordsTest::onTick(C_GameMode* gm) {
	syncModeFromSettings();
	syncCloneSubmodes();
	syncStructureMode();

	if (!GameData::canUseMoveKeys())
		return;

	if (GameData::isRightClickDown() && !hasClicked) {
		hasClicked = true;

		// In axe tool mode we only process clicks while holding a wooden axe
		if (axeToolMode && !isHoldingWoodenAxe())
			return;

		const vec3_ti block = g_Data.getClientInstance()->getPointerStruct()->block;
		if (block == vec3_ti(0, 0, 0))
			return;

		if (clickCount == 0) {
			C_Block* blk = gm->player->region->getBlock(block);
			if (blk == nullptr || blk->toLegacy() == nullptr) {
				g_Data.getGuiData()->displayClientMessageF("%s[AutoCords]%s First click is not a valid block.", RED, GRAY);
				return;
			}

			std::string name = blk->toLegacy()->name.getText();
			std::string lower = name;
			std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

			originIsStructureBlock = (lower.find("structure") != std::string::npos);
			bool isCommand = (lower.find("command") != std::string::npos);
			if (!originIsStructureBlock && !isCommand) {
				g_Data.getGuiData()->displayClientMessageF("%s[AutoCords]%s First click must be a command block or structure block.", RED, GRAY);
				return;
			}

			originBlock = block;
			clickedBlocks[0] = originBlock;
			clickCount = 1;

			if (teleportBackToOrigin && g_Data.getLocalPlayer() != nullptr) {
				savedPlayerPos = *g_Data.getLocalPlayer()->getPos();
				savedPlayerRot = g_Data.getLocalPlayer()->viewAngles;
				hasSavedPlayerState = true;
			}

			g_Data.getGuiData()->displayClientMessageF("%s[AutoCords]%s Origin (%s) saved at %d %d %d.",
				GREEN, GRAY, originIsStructureBlock ? "structure block" : "command block", block.x, block.y, block.z);
			return;
		}

		const int required = getRequiredClicks();
		if (clickCount >= required) {
			g_Data.getGuiData()->displayClientMessageF("%s[AutoCords]%s Already collected %d points for this mode. "
				"Disable/enable module to reset.",
				RED, GRAY, required);
			return;
		}

		// Save this point
		clickedBlocks[clickCount] = block;
		clickCount++;

		if (clickCount < required) {
			g_Data.getGuiData()->displayClientMessageF("%s[AutoCords]%s Point %d saved at %d %d %d.",
				GREEN, GRAY, clickCount, block.x, block.y, block.z);
			return;
		}

		std::string cmd = buildCommand(clickedBlocks, clickCount);
		std::string cmdCopy = cmd;
		Utils::setClipboardText(cmdCopy);

		if (!originIsStructureBlock) {
			g_Data.getGuiData()->displayClientMessageF("%s[AutoCords]%s Command copied to clipboard: %s%s",
				GREEN, GRAY, YELLOW, cmd.c_str());
		}

		if (giveBlockItem) {
			const std::string previousClipboard = cmdCopy;

			std::string snbt;
			if (originIsStructureBlock) {
				// Requires struct mode
				if (structLoad && clickCount >= 2)
					snbt = buildStructureSnbtLoad(clickedBlocks[0], clickedBlocks[1]);
				else if (structSave && clickCount >= 3)
					snbt = buildStructureSnbtSave(clickedBlocks[0], clickedBlocks[1], clickedBlocks[2]);
			} else {
				snbt = "{Count:1b,Name:\"minecraft:command_block\",tag:{Command:\"";
				for (char c : cmd) {
					if (c == '\\' || c == '"')
						snbt.push_back('\\');
					snbt.push_back(c);
				}
				snbt += "\",CustomName:\"\",ExecuteOnFirstTick:0b,LPCommandMode:0,LPCondionalMode:0b,LPRedstoneMode:0b,TickDelay:0,TrackOutput:1b,auto:0b,conditionMet:0b,id:\"CommandBlock\",powered:0b}}";
			}

			if (!snbt.empty()) {
				Utils::setClipboardText(snbt);

				std::string exec = std::string(1, cmdMgr->prefix) + "nbt load";
				std::vector<char> execBuf(exec.begin(), exec.end());
				execBuf.push_back('\0');
				cmdMgr->execute(execBuf.data());

				// Horion's nbt load forces count=64. AutoCords wants single blocks.
				//Unfortunately, this fix is not working yet.
				if (g_Data.getLocalPlayer() != nullptr) {
					auto item = g_Data.getLocalPlayer()->getSelectedItem();
					if (item != nullptr)
						item->count = 1;
				}

				Utils::setClipboardText(cmdCopy);
			}
		} else if (originIsStructureBlock) {
			// Safe mode: don't overwrite item. Provide chat-pasteable command.
			std::string snbt;
			if (structLoad && clickCount >= 2)
				snbt = buildStructureSnbtLoad(clickedBlocks[0], clickedBlocks[1]);
			else if (structSave && clickCount >= 3)
				snbt = buildStructureSnbtSave(clickedBlocks[0], clickedBlocks[1], clickedBlocks[2]);

			if (!snbt.empty()) {
				Utils::setClipboardText(snbt);
				g_Data.getGuiData()->displayClientMessageF("%s[AutoCords]%s Structure SNBT copied. Hold target item and run %s%cnbt load%s.",
					YELLOW, GRAY, WHITE, cmdMgr->prefix, GRAY);
			} else {
				g_Data.getGuiData()->displayClientMessageF("%s[AutoCords]%s Structure block detected, but Struct LOAD/SAVE not set.", YELLOW, GRAY);
			}
		}

		if (teleportBackToOrigin && hasSavedPlayerState && g_Data.getLocalPlayer() != nullptr) {
			g_Data.getLocalPlayer()->setPos(savedPlayerPos);
			g_Data.getLocalPlayer()->setRot(savedPlayerRot);
		}

		// Prepare for next command immediately
		resetSelection();
	}

	if (!GameData::isRightClickDown())
		hasClicked = false;
}


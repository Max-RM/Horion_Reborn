#pragma once
#include "Module.h"

class AutoCordsTest : public IModule {
public:
	enum class Mode {
		SetBlock = 0,
		Fill,
		Clone,
		TickingArea,
		TestForBlocks
	};

private:
	bool hasClicked = false;

	Mode currentMode = Mode::SetBlock;

	// Mutually exclusive mode settings (checkboxes)
	bool modeSetBlock = true;
	bool modeFill = false;
	bool modeClone = false;
	bool modeTickingArea = false;
	bool modeTestForBlocks = false;

	bool prevModeSetBlock = true;
	bool prevModeFill = false;
	bool prevModeClone = false;
	bool prevModeTickingArea = false;
	bool prevModeTestForBlocks = false;

	// Clone mask modes (mutually exclusive, but all-off allowed)
	bool cloneMaskFiltered = false;
	bool cloneMaskMasked = false;
	bool cloneMaskReplace = false;

	bool prevCloneMaskFiltered = false;
	bool prevCloneMaskMasked = false;
	bool prevCloneMaskReplace = false;

	// Clone move modes (mutually exclusive, but all-off allowed)
	bool cloneMoveForce = false;
	bool cloneMoveMove = false;
	bool cloneMoveNormal = false;

	bool prevCloneMoveForce = false;
	bool prevCloneMoveMove = false;
	bool prevCloneMoveNormal = false;

	// Optional behavior toggles
	bool teleportBackToOrigin = false;
	bool axeToolMode = false;
	bool giveBlockItem = false;

	// Console-only block suffixes (not saved in config)
	std::string setblockBlockText;
	std::string fillBlockText;

	// UI separators (non-functional)
    //int separatorModes = 0;

	// Structure block support (manual mode selection)
	bool structLoad = false;
	bool structSave = false;
	bool prevStructLoad = false;
	bool prevStructSave = false;

	int clickCount = 0;
	vec3_ti originBlock{};
	vec3_ti clickedBlocks[4]{};
	bool originIsStructureBlock = false;

	vec3_t savedPlayerPos{};
	vec2_t savedPlayerRot{};
	bool hasSavedPlayerState = false;

	int getRequiredClicks() const;
	std::string buildCommand(const vec3_ti* clicks, int count) const;
	std::string buildStructureSnbtLoad(const vec3_ti& origin, const vec3_ti& target) const;
	std::string buildStructureSnbtSave(const vec3_ti& origin, const vec3_ti& a, const vec3_ti& b) const;
	void resetSelection();
	void syncModeFromSettings();
	void syncCloneSubmodes();
	bool isHoldingWoodenAxe() const;
	void syncStructureMode();

public:
	AutoCordsTest();
	~AutoCordsTest();

	virtual const char* getModuleName() override;
	virtual void onEnable() override;
	virtual void onDisable() override;
	virtual void onTick(C_GameMode* gm) override;

	bool shouldAxeToolMode() const { return axeToolMode; }

	// Console sync helpers
	void setModeFromConsole(Mode m);
	void setSetblockBlockText(std::string txt) { setblockBlockText = std::move(txt); }
	void setFillBlockText(std::string txt) { fillBlockText = std::move(txt); }
	void setCloneMaskFromConsole(const std::string& mask);
	void setCloneMoveFromConsole(const std::string& move);
	void setStructModeFromConsole(const std::string& mode);

	void setTpBack(bool v) { teleportBackToOrigin = v; }
	void setAxeToolMode(bool v) { axeToolMode = v; }
	void setGiveBlockItem(bool v) { giveBlockItem = v; }

	// Rendering selection helpers
	virtual void onLevelRender() override;

private:
	bool renderSelection = true;
	float renderAlpha = 0.35f;
};


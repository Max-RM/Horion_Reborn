#include "Nbt.h"
#include "../../../SDK/Tag.h"
#include "../../../Utils/Utils.h"

Nbt::Nbt() : IModule(0, Category::PLAYER, "Print NBT tags, right click on mobs (Only works on local worlds!)") {
}

Nbt::~Nbt() {
}

const char* Nbt::getModuleName() {
	return ("Nbt");
}

void Nbt::onTick(C_GameMode* gm) {
	if (!GameData::canUseMoveKeys()) {
		return;
	}
	PointingStruct* pointingStruct = g_Data.getClientInstance()->getPointerStruct();
	
	if (GameData::isRightClickDown()) {  // && Utils::getClipboardText() != this->lastCopy) {
		if (pointingStruct->entityPtr != nullptr) {

			if (!(g_Data.getRakNetInstance()->serverIp.getTextLength() < 1))
				return;
			std::unique_ptr<CompoundTag> tag = std::make_unique<CompoundTag>();
			pointingStruct->entityPtr->save(tag.get());
			std::stringstream build;
			tag->write(build);
			auto str = build.str();
			if (this->lastCopy == str)
				return;
			this->lastCopy = str;
			Utils::setClipboardText(this->lastCopy);
			g_Data.getGuiData()->displayClientMessageF("%s%s", GREEN, "CompoundTag copied (full text is in clipboard):");
			C_GuiData* gui = g_Data.getGuiData();
			constexpr size_t kChunk = 512;
			for (size_t i = 0; i < str.size();) {
				const size_t end = (i + kChunk < str.size()) ? i + kChunk : str.size();
				std::string part = str.substr(i, end - i);
				gui->displayClientMessage(&part);
				i = end;
			}
		}
	}
}

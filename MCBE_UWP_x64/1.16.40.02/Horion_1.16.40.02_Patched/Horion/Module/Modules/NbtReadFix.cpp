#include "NbtReadFix.h"

NbtReadFix::NbtReadFix()
	: IModule(0, Category::PLAYER,
			   "When on, .nbt read exports full held-item SNBT. Off = tag-only (old behavior)") {
}

const char* NbtReadFix::getModuleName() {
	return "NbtReadFix";
}

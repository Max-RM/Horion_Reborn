#pragma once
#include "Module.h"

class NbtReadFix : public IModule {
public:
	NbtReadFix();
	virtual const char* getModuleName() override;
};

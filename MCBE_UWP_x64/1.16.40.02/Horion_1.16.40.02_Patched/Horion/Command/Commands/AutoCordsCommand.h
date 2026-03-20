#pragma once
#include "ICommand.h"

class AutoCordsCommand : public IMCCommand {
public:
	AutoCordsCommand();
	~AutoCordsCommand();

	virtual bool execute(std::vector<std::string>* args) override;
};


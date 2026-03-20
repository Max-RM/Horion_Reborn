#include "AutoCordsCommand.h"

#include <algorithm>
#include <sstream>

#include "../CommandMgr.h"
#include "../../Module/ModuleManager.h"
#include "../../Module/Modules/AutoCordsTest.h"

static std::string toLower(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

static std::string joinFrom(const std::vector<std::string>& args, size_t idx) {
	if (idx >= args.size())
		return "";
	std::ostringstream os;
	for (size_t i = idx; i < args.size(); i++) {
		if (i > idx) os << " ";
		os << args[i];
	}
	return os.str();
}

static bool parseOnOff(const std::string& s, bool& out) {
	auto v = toLower(s);
	if (v == "1" || v == "on" || v == "true" || v == "enable" || v == "enabled") {
		out = true;
		return true;
	}
	if (v == "0" || v == "off" || v == "false" || v == "disable" || v == "disabled") {
		out = false;
		return true;
	}
	return false;
}

AutoCordsCommand::AutoCordsCommand() : IMCCommand("ac", "Configure AutoCords", "<mode/settings> ...") {
	registerAlias("autocords");
}

AutoCordsCommand::~AutoCordsCommand() {
}

bool AutoCordsCommand::execute(std::vector<std::string>* args) {
	assertTrue(args->size() >= 2);

	auto mod = moduleMgr->getModule<AutoCordsTest>();
	if (mod == nullptr) {
		clientMessageF("%sAutoCords module not found", RED);
		return true;
	}

	std::string sub = toLower(args->at(1));

	// .ac setblock [block]
	if (sub == "setblock") {
		mod->setModeFromConsole(AutoCordsTest::Mode::SetBlock);
		mod->setSetblockBlockText(toLower(joinFrom(*args, 2)));
		clientMessageF("%sAutoCords: mode=setblock", GREEN);
		return true;
	}

	// .ac fill [block]
	if (sub == "fill") {
		mod->setModeFromConsole(AutoCordsTest::Mode::Fill);
		mod->setFillBlockText(toLower(joinFrom(*args, 2)));
		clientMessageF("%sAutoCords: mode=fill", GREEN);
		return true;
	}

	// .ac clone [filtered|masked|replace] [force|move|normal]
	if (sub == "clone") {
		mod->setModeFromConsole(AutoCordsTest::Mode::Clone);

		std::string mask = args->size() >= 3 ? toLower(args->at(2)) : "";
		std::string move = args->size() >= 4 ? toLower(args->at(3)) : "";

		if (mask == "filtered" || mask == "masked" || mask == "replace")
			mod->setCloneMaskFromConsole(mask);
		else
			mod->setCloneMaskFromConsole("");

		if (move == "force" || move == "move" || move == "normal")
			mod->setCloneMoveFromConsole(move);
		else
			mod->setCloneMoveFromConsole("");

		clientMessageF("%sAutoCords: mode=clone", GREEN);
		return true;
	}

	if (sub == "tickingarea") {
		mod->setModeFromConsole(AutoCordsTest::Mode::TickingArea);
		clientMessageF("%sAutoCords: mode=tickingarea", GREEN);
		return true;
	}

	if (sub == "testforblocks") {
		mod->setModeFromConsole(AutoCordsTest::Mode::TestForBlocks);
		clientMessageF("%sAutoCords: mode=testforblocks", GREEN);
		return true;
	}

	// Structure modes (manual)
	if (sub == "struct") {
		assertTrue(args->size() >= 3);
		std::string m = toLower(args->at(2));
		mod->setStructModeFromConsole(m); // load/save/off
		clientMessageF("%sAutoCords: struct=%s", GREEN, m.c_str());
		return true;
	}

	// Toggles: .ac tpback on/off, .ac axemode on/off, .ac giveitem on/off
	if (sub == "tpback" || sub == "axemode" || sub == "giveitem") {
		assertTrue(args->size() >= 3);
		bool v = false;
		assertTrue(parseOnOff(args->at(2), v));
		if (sub == "tpback") mod->setTpBack(v);
		if (sub == "axemode") mod->setAxeToolMode(v);
		if (sub == "giveitem") mod->setGiveBlockItem(v);
		clientMessageF("%sAutoCords: %s=%s", GREEN, sub.c_str(), v ? "on" : "off");
		return true;
	}

	clientMessageF("%sUnknown subcommand. Example: %cac setblock stone", RED, cmdMgr->prefix);
	return true;
}


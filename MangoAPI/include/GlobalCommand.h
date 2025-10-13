#pragma once

#include <functional>
#include <unordered_map>
#include <string>

using Command = std::function<void()>;

inline std::unordered_map<std::string, Command> dialogue_action_commands = {
	{"exit", [&]() { std::exit(0); } }
};

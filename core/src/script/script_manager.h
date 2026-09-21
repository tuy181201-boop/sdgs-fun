#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <nlohmann/json.hpp>
#include "script_handler.h"
namespace sdgs {
enum class ScriptLanguage
{
	UNKNOWN,
	LISP,
	FORTRAN,
	COBOL,
	BASIC,
	APL,
	SNOBOL,
	SIMULA,
	PASCAL,
	C,
	FORTH,
	RUST,
	ERLANG,
	LUA,
	FENNEL,
	MOONSCRIPT,
	LUAU,
	METALUA,
	GSL,
	LLVMLUA,
	CLIPS
};
class ScriptManager
{
public:
	void loadHandlers();
	void loadAllScripts(const std::string& scriptsDir);
	bool executeScript(const std::string& path, const nlohmann::json& context);
private:
	std::unordered_map<std::string, std::unique_ptr<ScriptHandler>> handlers_;
};
}
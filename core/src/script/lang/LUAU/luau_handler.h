#pragma once
#include "../script_handler.h"
namespace sdgs {
class LuauHandler : public ScriptHandler {
public:
    void execute(const std::string& code, const nlohmann::json& context) override;
};
}

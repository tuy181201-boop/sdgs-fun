#pragma once
#include "../script_handler.h"
namespace sdgs {
class LispHandler : public ScriptHandler {
public:
    void execute(const std::string& code, const nlohmann::json& context) override;
};
}

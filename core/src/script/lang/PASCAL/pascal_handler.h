#pragma once
#include "../script_handler.h"
namespace sdgs {
class PascalHandler : public ScriptHandler {
public:
    void execute(const std::string& code, const nlohmann::json& context) override;
};
}

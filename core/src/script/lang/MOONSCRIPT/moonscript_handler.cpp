#include "moonscript_handler.h"
#include "../llvm_compiler.h"
#include <iostream>
namespace sdgs {
void MoonscriptHandler::execute(const std::string& code, const nlohmann::json& context) {
    std::cout << "[MOONSCRIPT] Executing (stub -> LLVM JIT)" << std::endl;
    LLVMCompiler compiler;
    auto func = compiler.compileExpression("x + y * 2");
    if (func) {
        double result = func(5.0, 3.0);
        std::cout << "[MOONSCRIPT] JIT result: " << result << std::endl;
    }
}
}

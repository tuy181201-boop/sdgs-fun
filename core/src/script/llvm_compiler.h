#pragma once
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Support/TargetSelect.h>
#include <string>
#include <memory>
namespace sdgs {
class LLVMCompiler
{
public:
	LLVMCompiler();
	~LLVMCompiler();
	static void InitializeLLVM();
	using BinaryFunction = double (*)(double, double);
	BinaryFunction compileExpression(const std::string& expr);
private:
	std::unique_ptr<llvm::LLVMContext> context;
	std::unique_ptr<llvm::IRBuilder<>> builder;
	std::unique_ptr<llvm::Module> module;
	std::unique_ptr<llvm::orc::LLJIT> jit;
	llvm::Value* parseExpression(const std::string& expr, llvm::Function* func);
};
}
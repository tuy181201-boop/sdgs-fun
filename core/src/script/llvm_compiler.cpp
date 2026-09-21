#include "llvm_compiler.h"
#include <llvm/IR/Verifier.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/raw_ostream.h>
using namespace llvm;
using namespace llvm::orc;
namespace sdgs {
void LLVMCompiler::InitializeLLVM()
{
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
}
LLVMCompiler::LLVMCompiler()
	: context(std::make_unique<LLVMContext>()),
	  module(std::make_unique<Module>("sdgs_jit", *context)),
	  builder(std::make_unique<IRBuilder<>>(*context))
{
	auto J = LLJITBuilder().create();
	if (!J)
	{
		errs() << "Failed to create JIT\n";
		return;
	}
	jit = std::move(*J);
}
LLVMCompiler::~LLVMCompiler() = default;
llvm::Value* LLVMCompiler::parseExpression(const std::string& expr, llvm::Function* func)
{
	auto x = func->getArg(0);
	auto y = func->getArg(1);
	return builder->CreateFAdd(x, y, "addtmp");
}
LLVMCompiler::BinaryFunction LLVMCompiler::compileExpression(const std::string& expr)
{
	FunctionType* funcType = FunctionType::get(Type::getDoubleTy(*context),
		{Type::getDoubleTy(*context), Type::getDoubleTy(*context)}, false);
	Function* func = Function::Create(funcType, Function::ExternalLinkage, "jit_func", module.get());
	func->getArg(0)->setName("x");
	func->getArg(1)->setName("y");
	BasicBlock* entry = BasicBlock::Create(*context, "entry", func);
	builder->SetInsertPoint(entry);
	Value* result = parseExpression(expr, func);
	builder->CreateRet(result);
	verifyFunction(*func, &errs());
	ThreadSafeModule tsm(std::move(module), std::move(context));
	if (auto err = jit->addIRModule(std::move(tsm)))
	{
		errs() << "JIT add error\n";
		return nullptr;
	}
	auto sym = jit->lookup("jit_func");
	if (!sym)
	{
		errs() << "Symbol lookup error\n";
		return nullptr;
	}
	return (BinaryFunction)sym->getAddress();
}
}
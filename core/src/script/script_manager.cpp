#include "script_manager.h"
#include "lang/lisp_handler.h"
#include "lang/fortran_handler.h"
#include "lang/cobol_handler.h"
#include "lang/basic_handler.h"
#include "lang/apl_handler.h"
#include "lang/snobol_handler.h"
#include "lang/simula_handler.h"
#include "lang/pascal_handler.h"
#include "lang/c_handler.h"
#include "lang/forth_handler.h"
#include "lang/rust_handler.h"
#include "lang/erlang_handler.h"
#include "lang/lua_handler.h"
#include "lang/fennel_handler.h"
#include "lang/moonscript_handler.h"
#include "lang/luau_handler.h"
#include "lang/metalua_handler.h"
#include "lang/gsl_handler.h"
#include "lang/llvm_lua_handler.h"
#include "lang/clips_handler.h"
#include "lang/llvm_ir_handler.h"
#include <filesystem>
#include <iostream>
namespace sdgs {
void ScriptManager::loadHandlers() {
	handlers_[".lsp"]   = std::make_unique<LispHandler>();
	handlers_[".f"]     = std::make_unique<FortranHandler>();
	handlers_[".cbl"]   = std::make_unique<CobolHandler>();
	handlers_[".bas"]   = std::make_unique<BasicHandler>();
	handlers_[".apl"]   = std::make_unique<AplHandler>();
	handlers_[".sn"]    = std::make_unique<SnobolHandler>();
	handlers_[".sim"]   = std::make_unique<SimulaHandler>();
	handlers_[".pas"]   = std::make_unique<PascalHandler>();
	handlers_[".c"]     = std::make_unique<CHandler>();
	handlers_[".fth"]   = std::make_unique<ForthHandler>();
	handlers_[".rs"]    = std::make_unique<RustHandler>();
	handlers_[".erl"]   = std::make_unique<ErlangHandler>();
	handlers_[".lua"]   = std::make_unique<LuaHandler>();
	handlers_[".fnl"]   = std::make_unique<FennelHandler>();
	handlers_[".moon"]  = std::make_unique<MoonScriptHandler>();
	handlers_[".luau"]  = std::make_unique<LuauHandler>();
	handlers_[".meta"]  = std::make_unique<MetaLuaHandler>();
	handlers_[".gsl"]   = std::make_unique<GslHandler>();
	handlers_[".llvm"]  = std::make_unique<LlvmLuaHandler>();
	handlers_[".clp"]   = std::make_unique<ClipsHandler>();
	handlers_[".ll"]    = std::make_unique<LLVMIRHandler>();
	handlers_[".bc"]    = std::make_unique<LLVMIRHandler>();
	std::cout << "[ScriptManager] Registered " << handlers_.size() << " language handlers" << std::endl;
}
}
#include "python_bridge.h"
#include <pybind11/embed.h>
#include <pybind11/json.h>
#include <iostream>
namespace py = pybind11;
namespace sdgs {
PythonBridge::PythonBridge() = default;
PythonBridge::~PythonBridge() = default;
void PythonBridge::init(const std::string& worldPath)
{
	if (!Py_IsInitialized())
		py::initialize_interpreter();
	try
	{
		py::module_ sys = py::module_::import("sys");
		sys.attr("path").attr("insert")(0, worldPath + "/ai_core");
		py::module_::import("ai_core");
		std::cout << "[Python] AI Core loaded from " << worldPath << "/ai_core" << std::endl;
	}
	catch (const py::error_already_set& e)
	{
		std::cerr << "[Python] Failed to load AI Core: " << e.what() << std::endl;
	}
}
void PythonBridge::update()
{
	try
	{
		py::module_ ai_core = py::module_::import("ai_core");
		ai_core.attr("tick")();
	}
	catch (const py::error_already_set& e)
	{
		std::cerr << "[Python] Tick error: " << e.what() << std::endl;
	}
}
nlohmann::json PythonBridge::callFunction(const std::string& funcName, const nlohmann::json& args)
{
	try
	{
		py::module_ ai_core = py::module_::import("ai_core");
		py::object result = ai_core.attr(funcName.c_str())(args.dump());
		std::string json_str = py::cast<std::string>(result);
		return nlohmann::json::parse(json_str);
	}
	catch (const py::error_already_set& e)
	{
		std::cerr << "[Python] Call " << funcName << " error: " << e.what() << std::endl;
		return nlohmann::json::object();
	}
}
}
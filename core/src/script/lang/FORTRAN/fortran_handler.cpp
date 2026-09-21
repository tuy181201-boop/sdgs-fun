#include "fortran_handler.h"
#include "../physics/physics_bridge.h"   // ← подключаем физику
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>

namespace sdgs {

void FortranHandler::execute(const std::string& code, const nlohmann::json& context) {
    std::cout << "[FORTRAN] Executing physics from script..." << std::endl;

    // Пример: извлекаем данные из контекста (координаты и массы)
    // Предположим, что JSON содержит массивы:
    // context["voxel_x"], context["voxel_y"], context["voxel_z"], context["voxel_mass"]
    // и "voxel_size"
    if (!context.contains("voxel_x") || !context.contains("voxel_mass")) {
        std::cerr << "[FORTRAN] No voxel data in context!" << std::endl;
        return;
    }

    // Загружаем массивы из JSON
    auto x_arr = context["voxel_x"].get<std::vector<int>>();
    auto y_arr = context["voxel_y"].get<std::vector<int>>();
    auto z_arr = context["voxel_z"].get<std::vector<int>>();
    auto mass_arr = context["voxel_mass"].get<std::vector<uint64_t>>();

    int n = x_arr.size();
    double voxel_size = context.value("voxel_size", 0.000316227766); // √10/10 мм в метрах

    // Выходные параметры
    double total_mass, com_x, com_y, com_z;

    // Вызов Fortran-функции
    compute_center_of_mass(
        n,
        x_arr.data(),
        y_arr.data(),
        z_arr.data(),
        mass_arr.data(),
        voxel_size,
        &total_mass,
        &com_x,
        &com_y,
        &com_z
    );

    std::cout << "[FORTRAN] Total mass: " << total_mass << " kg\n";
    std::cout << "[FORTRAN] Center of mass: (" << com_x << ", " << com_y << ", " << com_z << ")\n";

    // Если нужно создать тело для дальнейшей симуляции
    body_t body = { total_mass, com_x, com_y, com_z, 0.0, 0.0, 0.0 };
    // Можно сохранить тело в глобальном реестре или вернуть в контекст
    // Например, context["body"] = {{"mass", total_mass}, ...};
}
} // namespace sdgs
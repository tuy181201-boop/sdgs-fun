#include "dna_parser.h"
#include <fstream>
#include <regex>
#include <algorithm>
namespace sdgs {
bool DNAParser::load(const std::string& path, WorldDNA& dna)
{
	std::ifstream file(path);
	if (!file) return false;
	std::string line;
	bool inBegin = false;
	while (std::getline(file, line))
	{
		std::string trimmed = line;
		trimmed.erase(0, trimmed.find_first_not_of(" \t"));
		if (trimmed.rfind("begin", 0) == 0) { inBegin = true; continue; }
		if (trimmed.rfind("end.", 0) == 0) { inBegin = false; continue; }
		if (inBegin && trimmed.find(":=") != std::string::npos)
			parseLine(trimmed, dna);
	}
	return true;
}
void DNAParser::parseLine(const std::string& line, WorldDNA& dna)
{
	std::regex assign_regex(R"((\w+\.?\w*)(?:\[(\d+)\])?\s*:=\s*([^;]+);)");
	std::smatch match;
	if (!std::regex_search(line, match, assign_regex)) return;
	std::string var = match[1];
	std::string valStr = match[3];
	valStr.erase(remove_if(valStr.begin(), valStr.end(), ::isspace), valStr.end());
	if (match[2].matched)
	{
		int idx = std::stoi(match[2]) - 1;
		float val = parseFloat(valStr);
		if (var == "Aesthetics.sky_color" && idx >= 0 && idx < 3) dna.aesthetics.sky_color[idx] = val;
		if (var == "Aesthetics.ground_color" && idx >= 0 && idx < 3) dna.aesthetics.ground_color[idx] = val;
		return;
	}
	float val = parseFloat(valStr);
	if (var == "Physics.gravity") dna.physics.gravity = val;
	else if (var == "Physics.friction") dna.physics.friction = val;
	else if (var == "Physics.air_resistance") dna.physics.air_resistance = val;
	else if (var == "Physics.elasticity") dna.physics.elasticity = val;
	else if (var == "Physics.wind_speed") dna.physics.wind_speed = val;
	else if (var == "Physics.water_viscosity") dna.physics.water_viscosity = val;
	else if (var == "Physics.magnetic_field") dna.physics.magnetic_field = val;
	else if (var == "Physics.light_speed") dna.physics.light_speed = val;
	else if (var == "Physics.space_curvature") dna.physics.space_curvature = val;
	else if (var == "Physics.dimensional_stability") dna.physics.dimensional_stability = val;
	else if (var == "Physics.gravity_direction") dna.physics.gravity_direction = val;
	else if (var == "Physics.planetary_radius") dna.physics.planetary_radius = val;
	else if (var == "Chemistry.reaction_rate") dna.chemistry.reaction_rate = val;
	else if (var == "Chemistry.autoignition_temp") dna.chemistry.autoignition_temp = val;
	else if (var == "Chemistry.ph_level") dna.chemistry.ph_level = val;
	else if (var == "Chemistry.oxygen_level") dna.chemistry.oxygen_level = val;
	else if (var == "Chemistry.pressure") dna.chemistry.pressure = val;
	else if (var == "Chemistry.ozone_layer") dna.chemistry.ozone_layer = val;
	else if (var == "Chemistry.radioactivity") dna.chemistry.radioactivity = val;
	else if (var == "Chemistry.carbon_cycle_rate") dna.chemistry.carbon_cycle_rate = val;
	else if (var == "Chemistry.nitrogen_fixation") dna.chemistry.nitrogen_fixation = val;
	else if (var == "Chemistry.salinity") dna.chemistry.salinity = val;
	else if (var == "Chemistry.water_hardness") dna.chemistry.water_hardness = val;
	else if (var == "Time.time_flow") dna.time.time_flow = val;
	else if (var == "Time.time_dilation") dna.time.time_dilation = val;
	else if (var == "Time.day_length") dna.time.day_length = val;
	else if (var == "Time.year_length") dna.time.year_length = val;
	else if (var == "Climate.temp_min") dna.climate.temp_min = val;
	else if (var == "Climate.temp_max") dna.climate.temp_max = val;
	else if (var == "Climate.rainfall") dna.climate.rainfall = val;
	else if (var == "Climate.humidity") dna.climate.humidity = val;
	else if (var == "Climate.seasonal_variation") dna.climate.seasonal_variation = val;
	else if (var == "Climate.wind_direction") dna.climate.wind_direction = val;
	else if (var == "Climate.cloud_cover") dna.climate.cloud_cover = val;
	else if (var == "Climate.atmospheric_density") dna.climate.atmospheric_density = val;
	else if (var == "Climate.albedo") dna.climate.albedo = val;
	else if (var == "Climate.dew_point") dna.climate.dew_point = val;
	else if (var == "Climate.wind_gustiness") dna.climate.wind_gustiness = val;
	else if (var == "Climate.volcanic_activity") dna.climate.volcanic_activity = val;
	else if (var == "Biome.forest") dna.biome.forest = val;
	else if (var == "Biome.desert") dna.biome.desert = val;
	else if (var == "Biome.mountains") dna.biome.mountains = val;
	else if (var == "Biome.water") dna.biome.water = val;
	else if (var == "Biome.urban") dna.biome.urban = val;
	else if (var == "Biome.swamp") dna.biome.swamp = val;
	else if (var == "Biome.tundra") dna.biome.tundra = val;
	else if (var == "Biome.ocean_depth") dna.biome.ocean_depth = val;
	else if (var == "Biome.cave_density") dna.biome.cave_density = val;
	else if (var == "Biome.biodiversity") dna.biome.biodiversity = val;
	else if (var == "Biome.species_richness") dna.biome.species_richness = val;
	else if (var == "Biome.coral_reef_density") dna.biome.coral_reef_density = val;
	else if (var == "Biome.kelp_forest_density") dna.biome.kelp_forest_density = val;
	else if (var == "NPC.aggressiveness") dna.npc.aggressiveness = val;
	else if (var == "NPC.curiosity") dna.npc.curiosity = val;
	else if (var == "NPC.loyalty") dna.npc.loyalty = val;
	else if (var == "NPC.intelligence") dna.npc.intelligence = val;
	else if (var == "NPC.strength") dna.npc.strength = val;
	else if (var == "NPC.speed") dna.npc.speed = val;
	else if (var == "NPC.social_cohesion") dna.npc.social_cohesion = val;
	else if (var == "NPC.fear_response") dna.npc.fear_response = val;
	else if (var == "NPC.reproduction_rate") dna.npc.reproduction_rate = val;
	else if (var == "NPC.population_growth") dna.npc.population_growth = val;
	else if (var == "NPC.maturity_age") dna.npc.maturity_age = val;
	else if (var == "NPC.aging_rate") dna.npc.aging_rate = val;
	else if (var == "Economy.resource_abundance") dna.economy.resource_abundance = val;
	else if (var == "Economy.resource_diversity") dna.economy.resource_diversity = val;
	else if (var == "Economy.trade_activity") dna.economy.trade_activity = val;
	else if (var == "Economy.trade_route_efficiency") dna.economy.trade_route_efficiency = val;
	else if (var == "Economy.currency_value") dna.economy.currency_value = val;
	else if (var == "Economy.tax_rate") dna.economy.tax_rate = val;
	else if (var == "Economy.inflation_rate") dna.economy.inflation_rate = val;
	else if (var == "Economy.black_market_activity") dna.economy.black_market_activity = val;
	else if (var == "Society.social_mobility") dna.society.social_mobility = val;
	else if (var == "Society.cultural_diversity") dna.society.cultural_diversity = val;
	else if (var == "Society.religion_influence") dna.society.religion_influence = val;
	else if (var == "Society.education_level") dna.society.education_level = val;
	else if (var == "Evolution.mutation_rate") dna.evolution.mutation_rate = val;
	else if (var == "Evolution.magic_potential") dna.evolution.magic_potential = val;
	else if (var == "Evolution.mana_regen") dna.evolution.mana_regen = val;
	else if (var == "Evolution.tech_level") dna.evolution.tech_level = val;
	else if (var == "Evolution.magic_chaos") dna.evolution.magic_chaos = val;
	else if (var == "Evolution.magic_stability") dna.evolution.magic_stability = val;
	else if (var == "Medicine.disease_resistance") dna.medicine.disease_resistance = val;
	else if (var == "Medicine.healing_rate") dna.medicine.healing_rate = val;
	else if (var == "Medicine.lifespan") dna.medicine.lifespan = val;
	else if (var == "Medicine.disease_lethality") dna.medicine.disease_lethality = val;
	else if (var == "Medicine.medical_technology") dna.medicine.medical_technology = val;
	else if (var == "Medicine.pandemic_probability") dna.medicine.pandemic_probability = val;
	else if (var == "Ecology.plant_growth_rate") dna.ecology.plant_growth_rate = val;
	else if (var == "Ecology.animal_reproduction") dna.ecology.animal_reproduction = val;
	else if (var == "Ecology.mutation_frequency") dna.ecology.mutation_frequency = val;
	else if (var == "Ecology.extinction_rate") dna.ecology.extinction_rate = val;
	else if (var == "Aesthetics.fog_density") dna.aesthetics.fog_density = val;
	else if (var == "Aesthetics.star_visibility") dna.aesthetics.star_visibility = val;
	else if (var == "Aesthetics.aurora_frequency") dna.aesthetics.aurora_frequency = val;
	else if (var == "Aesthetics.twilight_duration") dna.aesthetics.twilight_duration = val;
	else if (var == "Aesthetics.moon_phase") dna.aesthetics.moon_phase = val;
	else if (var == "Aesthetics.sun_angle") dna.aesthetics.sun_angle = val;
	else if (var == "Aesthetics.light_wavelength") dna.aesthetics.light_wavelength = val;
	else if (var == "Aesthetics.color_temperature") dna.aesthetics.color_temperature = val;
}
float DNAParser::parseFloat(const std::string& str) { return std::stof(str); }
}
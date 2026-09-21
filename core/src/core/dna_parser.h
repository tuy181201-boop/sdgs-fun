#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>
namespace sdgs {
struct PhysicsDNA { float gravity, friction, air_resistance, elasticity, wind_speed, water_viscosity, magnetic_field, light_speed, space_curvature, dimensional_stability, gravity_direction, planetary_radius; };
struct ChemistryDNA { float reaction_rate, autoignition_temp, ph_level, oxygen_level, pressure, ozone_layer, radioactivity, carbon_cycle_rate, nitrogen_fixation, salinity, water_hardness; };
struct TimeDNA { float time_flow, time_dilation, day_length, year_length; };
struct ClimateDNA { float temp_min, temp_max, rainfall, humidity, seasonal_variation, wind_direction, cloud_cover, atmospheric_density, albedo, dew_point, wind_gustiness, volcanic_activity; };
struct BiomeDNA { float forest, desert, mountains, water, urban, swamp, tundra, ocean_depth, cave_density, biodiversity, species_richness, coral_reef_density, kelp_forest_density; };
struct NPCDNA { float aggressiveness, curiosity, loyalty, intelligence, strength, speed, social_cohesion, fear_response, reproduction_rate, population_growth, maturity_age, aging_rate; };
struct EconomyDNA { float resource_abundance, resource_diversity, trade_activity, trade_route_efficiency, currency_value, tax_rate, inflation_rate, black_market_activity; };
struct SocietyDNA { float social_mobility, cultural_diversity, religion_influence, education_level; };
struct EvolutionDNA { float mutation_rate, magic_potential, mana_regen, tech_level, magic_chaos, magic_stability; };
struct MedicineDNA { float disease_resistance, healing_rate, lifespan, disease_lethality, medical_technology, pandemic_probability; };
struct EcologyDNA { float plant_growth_rate, animal_reproduction, mutation_frequency, extinction_rate; };
struct AestheticsDNA { float sky_color[3], ground_color[3], fog_density, star_visibility, aurora_frequency, twilight_duration, moon_phase, sun_angle, light_wavelength, color_temperature; };
struct WorldDNA
{
	PhysicsDNA physics;
	ChemistryDNA chemistry;
	TimeDNA time;
	ClimateDNA climate;
	BiomeDNA biome;
	NPCDNA npc;
	EconomyDNA economy;
	SocietyDNA society;
	EvolutionDNA evolution;
	MedicineDNA medicine;
	EcologyDNA ecology;
	AestheticsDNA aesthetics;
};
class DNAParser
{
public:
	bool load(const std::string& path, WorldDNA& dna);
private:
	void parseLine(const std::string& line, WorldDNA& dna);
	float parseFloat(const std::string& str);
	void parseArray(const std::string& var, const std::string& value, float* arr, int size);
};
}
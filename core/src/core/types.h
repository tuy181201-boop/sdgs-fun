#pragma once
#include <cstdint>
#include <array>
#include <cmath>
#include <string>
namespace sdgs {
struct PackedPosition
{
	uint8_t x_local[3];
	uint8_t x_chunk;
	uint8_t y_local[3];
	uint8_t y_chunk;
	uint8_t z_local[3];
	uint8_t z_chunk;
	inline double toDoubleX() const
	{
		uint32_t local = (uint32_t)x_local[0] | ((uint32_t)x_local[1] << 8) | ((uint32_t)x_local[2] << 16);
		return (double)x_chunk * 16777216.0 + (double)local;
	}
	inline double toDoubleY() const
	{
		uint32_t local = (uint32_t)y_local[0] | ((uint32_t)y_local[1] << 8) | ((uint32_t)y_local[2] << 16);
		return (double)y_chunk * 16777216.0 + (double)local;
	}
	inline double toDoubleZ() const
	{
		uint32_t local = (uint32_t)z_local[0] | ((uint32_t)z_local[1] << 8) | ((uint32_t)z_local[2] << 16);
		return (double)z_chunk * 16777216.0 + (double)local;
	}
	static PackedPosition fromDouble(double x, double y, double z)
	{
		const double CHUNK_SIZE = 16777216.0;
		PackedPosition pos;
		double x_chunk_d = floor(x / CHUNK_SIZE);
		pos.x_chunk = (uint8_t)x_chunk_d;
		uint32_t x_local = (uint32_t)(x - x_chunk_d * CHUNK_SIZE);
		pos.x_local[0] = x_local & 0xFF;
		pos.x_local[1] = (x_local >> 8) & 0xFF;
		pos.x_local[2] = (x_local >> 16) & 0xFF;
		double y_chunk_d = floor(y / CHUNK_SIZE);
		pos.y_chunk = (uint8_t)y_chunk_d;
		uint32_t y_local = (uint32_t)(y - y_chunk_d * CHUNK_SIZE);
		pos.y_local[0] = y_local & 0xFF;
		pos.y_local[1] = (y_local >> 8) & 0xFF;
		pos.y_local[2] = (y_local >> 16) & 0xFF;
		double z_chunk_d = floor(z / CHUNK_SIZE);
		pos.z_chunk = (uint8_t)z_chunk_d;
		uint32_t z_local = (uint32_t)(z - z_chunk_d * CHUNK_SIZE);
		pos.z_local[0] = z_local & 0xFF;
		pos.z_local[1] = (z_local >> 8) & 0xFF;
		pos.z_local[2] = (z_local >> 16) & 0xFF;
		return pos;
	}
	bool operator==(const PackedPosition& other) const
	{
		return memcmp(this, &other, sizeof(PackedPosition)) == 0;
	}
};
struct PackedRotation
{
	uint8_t pitch;
	uint8_t yaw;
	uint8_t roll;
	inline float toDegreesPitch() const { return (float)pitch * 360.0f / 256.0f; }
	inline float toDegreesYaw() const   { return (float)yaw * 360.0f / 256.0f; }
	inline float toDegreesRoll() const  { return (float)roll * 360.0f / 256.0f; }
	static PackedRotation fromDegrees(float pitchDeg, float yawDeg, float rollDeg)
	{
		PackedRotation rot;
		rot.pitch = (uint8_t)(pitchDeg / 360.0f * 256.0f);
		rot.yaw   = (uint8_t)(yawDeg / 360.0f * 256.0f);
		rot.roll  = (uint8_t)(rollDeg / 360.0f * 256.0f);
		return rot;
	}
};
#pragma pack(push, 1)
struct PackedTransform
{
	uint8_t x_local[3];
	uint8_t x_chunk;
	uint8_t y_local[3];
	uint8_t y_chunk;
	uint8_t z_local[3];
	uint8_t z_chunk;
	uint8_t pitch;
	uint8_t yaw;
	uint8_t roll;
	uint8_t uid[16];
};
#pragma pack(pop)
static_assert(sizeof(PackedTransform) == 37, "PackedTransform must be 37 bytes");
}
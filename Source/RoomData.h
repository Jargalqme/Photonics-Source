#pragma once
#include <string>
#include <vector>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

// Objects types that can be placed in a room
enum class ObjectType
{
	Pillar,
	Wall,
	Platform,
	Cover,
	Gate,
	SpawnPoint
};

// Single object placed in the room
struct PlacedObject
{
	ObjectType type = ObjectType::Pillar;
	Vector3 position = Vector3::Zero;
	Vector3 rotation = Vector3::Zero;
	Vector3 scale = Vector3::One;
	Color color = Color(1.0f, 1.0f, 1.0f, 1.0f);
};

// An enemy spawn point with wave info
struct SpawnPoint
{
	Vector3 position = Vector3::Zero;
	std::string enemyType = "troop";
	int wave = 1;
};

// Complete room definition
struct RoomData
{
	std::string name = "untitled";
	float floorWidth = 200.0f;
	float floorDepth = 200.0f;
	Vector3 playerStart = Vector3::Zero;
	Vector3 entryGatePos = Vector3::Zero;
	Vector3 exitGatePos = Vector3::Zero;

	std::vector<PlacedObject> objects;
	std::vector<SpawnPoint> spawns;

	// File I/O
	bool save(const std::string& filepath) const;
	bool load(const std::string& filepath);
};
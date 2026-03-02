#include "pch.h"
#include "RoomData.h"
#include "../ThirdParty/json.hpp"
#include <fstream>

using json = nlohmann::json;

// Helper: ObjectType <-> string
static const char* ObjectTypeToString(ObjectType type)
{
	switch (type)
	{
	case ObjectType::Pillar: return "pillar";
	case ObjectType::Wall:   return "wall";
	case ObjectType::Platform: return "platform";
	case ObjectType::Cover: return "cover";
	case ObjectType::Gate: return "gate";
	case ObjectType::SpawnPoint: return "spawnpoint";
	default: return "pillar";
	}
}

static ObjectType StringToObjectType(const std::string& str)
{
	if (str == "wall") return ObjectType::Wall;
	if (str == "platform") return ObjectType::Platform;
	if (str == "cover") return ObjectType::Cover;
	if (str == "gate") return ObjectType::Gate;
	if (str == "spawnpoint") return ObjectType::SpawnPoint;
	return ObjectType::Pillar;
}

// Helper: Vector3 <-> JSON array [x, y, z]
static json Vec3ToJson(const Vector3& v)
{
	return { v.x, v.y, v.z };
}

static Vector3 JsonToVec3(const json& j)
{
	return Vector3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
}

// Helper: Color <-> JSON array [r, g, b, a]
static json ColorToJson(const Color& c)
{
	return { c.x, c.y, c.z, c.w };
}

static Color JsonToColor(const json& j)
{
	return Color(j[0].get<float>(), j[1].get<float>(), j[2].get<float>(), j[3].get<float>());
}

 // ---------------------------------------------------------------------------
 // Save
 // ---------------------------------------------------------------------------
 bool RoomData::save(const std::string & filepath) const
 {
   json j;
   j["name"] = name;
   j["floor_width"] = floorWidth;
   j["floor_depth"] = floorDepth;
   j["player_start"] = Vec3ToJson(playerStart);
   j["entry_gate"] = Vec3ToJson(entryGatePos);
   j["exit_gate"] = Vec3ToJson(exitGatePos);

   // Objects
   json objArray = json::array();
   for (const auto& obj : objects)
   {
           json o;
           o["type"] = ObjectTypeToString(obj.type);
           o["position"] = Vec3ToJson(obj.position);
           o["rotation"] = Vec3ToJson(obj.rotation);
           o["scale"] = Vec3ToJson(obj.scale);
           o["color"] = ColorToJson(obj.color);
           objArray.push_back(o);
   }
   j["objects"] = objArray;

   // Spawn points
   json spawnArray = json::array();
   for (const auto& sp : spawns)
   {
           json s;
           s["position"] = Vec3ToJson(sp.position);
           s["enemy_type"] = sp.enemyType;
           s["wave"] = sp.wave;
           spawnArray.push_back(s);
   }
   j["spawns"] = spawnArray;

   // Write to file
   std::ofstream file(filepath);
   if (!file.is_open())
           return false;

   file << j.dump(4);
   return true;
 }

 // ---------------------------------------------------------------------------
 // Load
 // ---------------------------------------------------------------------------
 bool RoomData::load(const std::string & filepath)
 {
   std::ifstream file(filepath);
   if (!file.is_open())
           return false;

   json j;
   try
   {
           file >> j;
   }
   catch (...)
   {
           return false;
   }

   name = j.value("name", "untitled");
   floorWidth = j.value("floor_width", 200.0f);
   floorDepth = j.value("floor_depth", 200.0f);

   if (j.contains("player_start")) playerStart = JsonToVec3(j["player_start"]);
   if (j.contains("entry_gate"))   entryGatePos = JsonToVec3(j["entry_gate"]);
   if (j.contains("exit_gate"))    exitGatePos = JsonToVec3(j["exit_gate"]);

   // Objects
   objects.clear();
   if (j.contains("objects"))
   {
           for (const auto& o : j["objects"])
           {
                   PlacedObject obj;
                   obj.type = StringToObjectType(o.value("type", "pillar"));
                   obj.position = JsonToVec3(o["position"]);
                   obj.rotation = JsonToVec3(o["rotation"]);
                   obj.scale = JsonToVec3(o["scale"]);
                   obj.color = JsonToColor(o["color"]);
                   objects.push_back(obj);
           }
   }

   // Spawn points
   spawns.clear();
   if (j.contains("spawns"))
   {
           for (const auto& s : j["spawns"])
           {
                   SpawnPoint sp;
                   sp.position = JsonToVec3(s["position"]);
                   sp.enemyType = s.value("enemy_type", "troop");
                   sp.wave = s.value("wave", 1);
                   spawns.push_back(sp);
           }
   }

   return true;
}
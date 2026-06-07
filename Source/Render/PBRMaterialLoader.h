#pragma once

#include "ImportedModel.h"

#include <filesystem>
#include <string>

namespace PBRMaterialLoader
{
	bool ApplyAmbientCGTextureSet(
		ImportedModelData& data,
		uint32_t materialIndex,
		const std::filesystem::path& materialDirectory,
		std::string* outError = nullptr);
}
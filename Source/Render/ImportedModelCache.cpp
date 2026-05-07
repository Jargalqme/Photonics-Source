#include "pch.h"
#include "ImportedModelCache.h"

#include "AssimpModelImporter.h"

namespace
{
    void TraceLine(const std::string& text)
    {
        OutputDebugStringA(text.c_str());
        OutputDebugStringA("\n");
    }
}

void ImportedModelCache::initialize(ID3D11Device* device)
{
    m_device = device;
}

void ImportedModelCache::finalize()
{
    m_models.clear();
    m_device = nullptr;
}

const ImportedModel* ImportedModelCache::get(const std::string& path)
{
    auto it = m_models.find(path);
    if (it != m_models.end())
    {
        return it->second.get();
    }

    if (!m_device)
    {
        return nullptr;
    }

    ImportedModelData data;
    std::string error;
    if (!AssimpModelImporter::LoadImportedModelData(path, data, &error))
    {
        TraceLine("[ImportedModelCache] Failed to load " + path + ": " + error);
        return nullptr;
    }

    auto model = std::make_unique<ImportedModel>();
    if (!model->initialize(m_device, std::move(data)))
    {
        TraceLine("[ImportedModelCache] Failed to create GPU buffers for " + path);
        return nullptr;
    }

    const ImportedModel* result = model.get();
    m_models.emplace(path, std::move(model));

    TraceLine("[ImportedModelCache] Loaded " + path
        + " vertices=" + std::to_string(result->vertexCount())
        + " indices=" + std::to_string(result->indexCount())
        + " submeshes=" + std::to_string(result->submeshes().size())
        + " textures=" + std::to_string(result->textures().size()));

    return result;
}

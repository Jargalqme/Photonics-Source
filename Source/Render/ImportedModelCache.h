#pragma once

#include "ImportedModel.h"

#include <memory>
#include <string>
#include <unordered_map>

class ImportedModelCache
{
public:
    void initialize(ID3D11Device* device);
    void finalize();

    const ImportedModel* get(const std::string& path);

private:
    ID3D11Device* m_device = nullptr;
    std::unordered_map<std::string, std::unique_ptr<ImportedModel>> m_models;
};

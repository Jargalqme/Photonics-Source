#pragma once

#include <SimpleMath.h>
#include <string>

class ImportedModel;
class RenderCommandQueue;
struct SceneContext;

struct ImportedRifleViewmodelSettings
{
    float targetLength = 1.68f;
    DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3(-0.015f, -0.195f, 0.180f);
    DirectX::SimpleMath::Vector3 rotationDegrees = DirectX::SimpleMath::Vector3(0.0f, 180.0f, 0.0f);
};

// First-person imported weapon model.
class PlayerViewmodel
{
public:
    bool loadImportedRifle(SceneContext& context, const std::string& path);
    void submit(RenderCommandQueue& queue, const DirectX::SimpleMath::Matrix& rootWorld) const;
    DirectX::SimpleMath::Matrix buildModelWorldMatrix(const DirectX::SimpleMath::Matrix& rootWorld) const;
    DirectX::SimpleMath::Vector3 getMuzzleLocalPosition() const;
    void finalize();

    bool hasImportedRifle() const { return m_importedRifle != nullptr; }
    ImportedRifleViewmodelSettings& settings() { return m_importedRifleSettings; }
    const ImportedRifleViewmodelSettings& settings() const { return m_importedRifleSettings; }
    void resetSettings();

private:
    const ImportedModel* m_importedRifle = nullptr;
    ImportedRifleViewmodelSettings m_importedRifleSettings;
    DirectX::SimpleMath::Vector3 m_importedRifleCenter = DirectX::SimpleMath::Vector3::Zero;
    float m_importedRifleLongestSide = 1.0f;

    static constexpr float kFallbackMuzzleZ = 0.3f;
};

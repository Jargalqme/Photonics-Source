//=============================================================================
// @brief    Scene-level rendering orchestration.
//           Owns post-process effects (Bloom, scene-copy) and exposes a
//           single render entry point. Sits between the low-level Renderer
//           (frame orchestration) and the gameplay scenes.
//           Pattern modeled after Hazel's SceneRenderer at appropriate scale.
//=============================================================================
#pragma once

#include "Core/DeviceResources.h"
#include <memory>

class Bloom;
class SceneCopyPass;

class SceneRenderer
{
public:
    SceneRenderer(DX::DeviceResources* deviceResources);
    ~SceneRenderer();

    SceneRenderer(SceneRenderer&&) = default;
    SceneRenderer& operator=(SceneRenderer&&) = default;
    SceneRenderer(SceneRenderer const&) = delete;
    SceneRenderer& operator=(SceneRenderer const&) = delete;

    void createDeviceDependentResources();
    void createWindowSizeDependentResources(int width, int height);
    void onDeviceLost();

    // Run the post-process chain. Source is the HDR scene SRV;
    // destination is the BGRA backbuffer RTV.
    void renderPostProcess(
        ID3D11ShaderResourceView* sceneSRV,
        ID3D11RenderTargetView* backbuffer);

    // Expose Bloom for DebugUI sliders. Owned, non-transferring.
    Bloom* getBloom() { return m_bloom.get(); }

private:
    DX::DeviceResources* m_deviceResources;
    std::unique_ptr<Bloom>         m_bloom;
    std::unique_ptr<SceneCopyPass> m_copyPass;
};

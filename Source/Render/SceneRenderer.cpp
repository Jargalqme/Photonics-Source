#include "pch.h"
#include "Render/SceneRenderer.h"
#include "Render/Bloom.h"
#include "Render/SceneCopyPass.h"

SceneRenderer::SceneRenderer(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
    , m_bloom(std::make_unique<Bloom>(deviceResources))
    , m_copyPass(std::make_unique<SceneCopyPass>(deviceResources))
{
}

// Destructor defined here so unique_ptr<Bloom> / unique_ptr<SceneCopyPass>
// see the full types. Header forward-declares only.
SceneRenderer::~SceneRenderer() = default;

void SceneRenderer::createDeviceDependentResources()
{
    m_bloom->createDeviceDependentResources();
    m_copyPass->createDeviceDependentResources();
}

void SceneRenderer::createWindowSizeDependentResources(int width, int height)
{
    m_bloom->createWindowSizeDependentResources(width, height);
    // SceneCopyPass has no window-size-dependent resources.
}

void SceneRenderer::onDeviceLost()
{
    m_bloom->finalize();
    m_copyPass->finalize();
}

void SceneRenderer::renderPostProcess(
    ID3D11ShaderResourceView* sceneSRV,
    ID3D11RenderTargetView* backbuffer)
{
    // Bloom is the only effect that toggles. The copy pass is always the
    // final step that lands the HDR scene (or post-bloom result) in the
    // BGRA backbuffer.
    if (m_bloom->isEnabled())
    {
        m_bloom->render(sceneSRV);
        m_copyPass->process(m_bloom->getOutputSRV(), backbuffer);
    }
    else
    {
        m_copyPass->process(sceneSRV, backbuffer);
    }
}
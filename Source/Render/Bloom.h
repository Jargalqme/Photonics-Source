#pragma once

#include "Core/DeviceResources.h"

class Bloom
{
public:
    Bloom(DX::DeviceResources* deviceResources);
    ~Bloom() = default;

    void createDeviceDependentResources();
    void createWindowSizeDependentResources(int width, int height);
    void finalize();

    void render(ID3D11ShaderResourceView* sceneSRV);

    ID3D11ShaderResourceView* getOutputSRV() { return m_compositeSRV.Get(); }

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

    // ImGuiスライダー用ポインタ
    bool* getEnabledPtr() { return &m_enabled; }
    float* getThresholdPtr() { return &m_threshold; }
    float* getKneePtr() { return &m_knee; }
    float* getIntensityPtr() { return &m_intensity; }
    float* getExposurePtr() { return &m_exposure; }

private:
    static constexpr int MIP_COUNT = 5;

    void renderFullscreenPass(ID3D11PixelShader* ps,
        ID3D11RenderTargetView* outputRTV,
        ID3D11ShaderResourceView* input0,
        ID3D11ShaderResourceView* input1,
        UINT width, UINT height);

    DX::DeviceResources* m_deviceResources;

    UINT m_targetWidth = 0;
    UINT m_targetHeight = 0;

    // ブルームミップチェーン（5レベル）
    Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_mipTextures[MIP_COUNT];
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   m_mipRTVs[MIP_COUNT];
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_mipSRVs[MIP_COUNT];
    UINT m_mipWidths[MIP_COUNT] = {};
    UINT m_mipHeights[MIP_COUNT] = {};

    // 合成出力（フル解像度、HDR）
    Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_compositeTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   m_compositeRTV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_compositeSRV;

    // シェーダー
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_fullscreenVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_prefilterPS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_downsamplePS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_upsamplePS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_compositePS;

    // 共有リソース
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_constantBuffer;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;
    Microsoft::WRL::ComPtr<ID3D11BlendState>   m_additiveBlend;

    // パラメータ
    bool  m_enabled = false;
    float m_threshold = 1.0f;
    float m_knee = 0.5f;
    float m_intensity = 1.0f;
    float m_exposure = 1.0f;

    // HLSL 定数バッファと一致
    struct BloomParamsCB
    {
        DirectX::XMFLOAT2 texelSize;
        float sampleScale;
        float padding1;
        DirectX::XMFLOAT4 threshold;
        float bloomIntensity;
        float exposure;
        DirectX::XMFLOAT2 padding2;
    };
};

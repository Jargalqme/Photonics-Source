//=============================================================================
/// @file    BulletRenderer.h
/// @brief   弾の描画（プレイヤー: シリンダー軌跡、敵: 六角形オーブ）
//=============================================================================
#pragma once
#include "DeviceResources.h"

class BulletPool;

class BulletRenderer
{
public:
    BulletRenderer(DX::DeviceResources* deviceResources);
    ~BulletRenderer() = default;

    void initialize();
    void update(float deltaTime);
    void render(BulletPool* bulletPool,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const DirectX::SimpleMath::Vector3& cameraPosition);
    void finalize();

private:
    // --- 定数 ---
    static constexpr uint32_t MAX_BOLTS = 256;
    static constexpr uint32_t VERTS_PER_BOLT = 108;
    static constexpr uint32_t VERTS_PER_ORB  = 6;

    // プレイヤー弾の外観
    static constexpr float PLAYER_BOLT_LENGTH = 1.0f;
    static constexpr float PLAYER_BOLT_WIDTH  = 0.05f;

    // 敵弾の外観
    static constexpr float ENEMY_ORB_RADIUS   = 0.5f;

    // HLSL と完全一致（48 バイト）
    struct BoltData
    {
        DirectX::SimpleMath::Vector3 position;
        float speed;
        DirectX::SimpleMath::Vector3 direction;
        float age;
        DirectX::SimpleMath::Vector4 color;
    };

    // HLSL と完全一致（96 バイト）
    struct BoltCB
    {
        DirectX::SimpleMath::Matrix viewProjection;
        DirectX::SimpleMath::Vector3 cameraPosition;
        float time;
        float boltLength;
        float boltWidth;
        uint32_t boltCount;
        float padding;
    };

    // デバイス（非所有）
    DX::DeviceResources* m_deviceResources;
    float m_time = 0.0f;

    // GPU リソース
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_boltBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_boltSRV;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
    BoltData m_boltData[MAX_BOLTS];

    // シェーダー — プレイヤー弾
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_playerPS;

    // シェーダー — 敵弾
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_orbVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_enemyPS;

    // レンダーステート（共通）
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
};

#include "pch.h"
#include "ImportedModel.h"
#include "WICTextureLoader.h"

namespace
{
    void TraceLine(const std::string& text)
    {
        OutputDebugStringA(text.c_str());
        OutputDebugStringA("\n");
    }

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCompressedTextureSRV(
        ID3D11Device* device,
        const ImportedTextureData& texture)
    {
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
        if (texture.bytes.empty())
        {
            return srv;
        }

        const auto loadFlags = texture.srgb
            ? DirectX::WIC_LOADER_FORCE_SRGB
            : DirectX::WIC_LOADER_IGNORE_SRGB;

        const HRESULT hr = DirectX::CreateWICTextureFromMemoryEx(
            device,
            texture.bytes.data(),
            texture.bytes.size(),
            0,
            D3D11_USAGE_DEFAULT,
            D3D11_BIND_SHADER_RESOURCE,
            0,
            0,
            loadFlags,
            nullptr,
            srv.ReleaseAndGetAddressOf());

        if (FAILED(hr))
        {
            TraceLine("[ImportedModel] Failed to decode texture: " + texture.source);
        }

        return srv;
    }

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateRawTextureSRV(
        ID3D11Device* device,
        const ImportedTextureData& texture)
    {
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;

        if (texture.width == 0 ||
            texture.height == 0 ||
            texture.bytes.size() !=
                static_cast<size_t>(texture.width) *
                static_cast<size_t>(texture.height) * 4)
        {
            return srv;
        }

        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = texture.width;
        textureDesc.Height = texture.height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = texture.srgb
            ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
            : DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA initialData = {};
        initialData.pSysMem = texture.bytes.data();
        initialData.SysMemPitch = texture.width * 4;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> d3dTexture;
        const HRESULT textureHr = device->CreateTexture2D(
            &textureDesc,
            &initialData,
            d3dTexture.ReleaseAndGetAddressOf());
        if (FAILED(textureHr))
        {
            TraceLine("[ImportedModel] Failed to create raw texture: " + texture.source);
            return srv;
        }

        const HRESULT srvHr = device->CreateShaderResourceView(
            d3dTexture.Get(),
            nullptr,
            srv.ReleaseAndGetAddressOf());
        if (FAILED(srvHr))
        {
            TraceLine("[ImportedModel] Failed to create raw texture SRV: " + texture.source);
        }

        return srv;
    }
}

bool ImportedModel::initialize(ID3D11Device* device, ImportedModelData data)
{
    finalize();

    if (!device || data.vertices.empty() || data.indices.empty())
    {
        return false;
    }

    const size_t vertexBufferBytes = data.vertices.size() * sizeof(ImportedModelVertex);
    const size_t indexBufferBytes = data.indices.size() * sizeof(uint32_t);
    if (vertexBufferBytes > UINT32_MAX || indexBufferBytes > UINT32_MAX)
    {
        return false;
    }

    D3D11_BUFFER_DESC vertexDesc = {};
    vertexDesc.ByteWidth = static_cast<UINT>(vertexBufferBytes);
    vertexDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = data.vertices.data();

    DX::ThrowIfFailed(device->CreateBuffer(
        &vertexDesc,
        &vertexData,
        m_vertexBuffer.ReleaseAndGetAddressOf()));

    D3D11_BUFFER_DESC indexDesc = {};
    indexDesc.ByteWidth = static_cast<UINT>(indexBufferBytes);
    indexDesc.Usage = D3D11_USAGE_DEFAULT;
    indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = data.indices.data();

    DX::ThrowIfFailed(device->CreateBuffer(
        &indexDesc,
        &indexData,
        m_indexBuffer.ReleaseAndGetAddressOf()));

    m_textureSRVs.resize(data.textures.size());
    for (size_t i = 0; i < data.textures.size(); ++i)
    {
        const ImportedTextureData& texture = data.textures[i];
        m_textureSRVs[i] = texture.compressed
            ? CreateCompressedTextureSRV(device, texture)
            : CreateRawTextureSRV(device, texture);
    }

    m_data = std::move(data);
    return true;
}

void ImportedModel::finalize()
{
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
    m_textureSRVs.clear();
    m_data = ImportedModelData{};
}

ID3D11ShaderResourceView* ImportedModel::textureSRV(int32_t textureIndex) const
{
    if (textureIndex < 0 ||
        static_cast<size_t>(textureIndex) >= m_textureSRVs.size())
    {
        return nullptr;
    }

    return m_textureSRVs[textureIndex].Get();
}

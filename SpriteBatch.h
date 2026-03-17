#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <string>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

class SpriteBatch {
public:
    struct Vertex {
        XMFLOAT2 pos;
        XMFLOAT2 tex;
    };

    SpriteBatch();
    bool Initialize(ID3D11Device* device);
    
    void Begin(ID3D11DeviceContext* context, int screenWidth, int screenHeight);
    void Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv, float x, float y, float w, float h, float srcX, float srcY, float srcW, float srcH);
    void End();

private:
    ComPtr<ID3D11VertexShader> m_vs;
    ComPtr<ID3D11PixelShader> m_ps;
    ComPtr<ID3D11InputLayout> m_layout;
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11SamplerState> m_sampler;
    ComPtr<ID3D11Buffer> m_constantBuffer;
    ComPtr<ID3D11BlendState> m_blendState;
    ComPtr<ID3D11RasterizerState> m_rasterState;
    ComPtr<ID3D11DepthStencilState> m_depthStencilState;

    struct CB {
        XMMATRIX projection;
    };
};

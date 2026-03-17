#include "SpriteBatch.h"
#include <d3dcompiler.h>

const char* shaderCode = R"(
cbuffer cb : register(b0) {
    row_major matrix projection;
};
struct VS_INPUT {
    float2 pos : POSITION;
    float2 tex : TEXCOORD;
};
struct PS_INPUT {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
};
PS_INPUT VS(VS_INPUT input) {
    PS_INPUT output;
    output.pos = mul(float4(input.pos, 0.0f, 1.0f), projection);
    output.tex = input.tex;
    return output;
}
Texture2D shaderTexture : register(t0);
SamplerState sampleState : register(s0);
float4 PS(PS_INPUT input) : SV_Target {
    return shaderTexture.Sample(sampleState, input.tex);
}
)";

SpriteBatch::SpriteBatch() {}

bool SpriteBatch::Initialize(ID3D11Device* device) {
    ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;
    HRESULT hr = D3DCompile(shaderCode, strlen(shaderCode), nullptr, nullptr, nullptr, "VS", "vs_4_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "VS Error", MB_ICONERROR);
        return false;
    }
    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vs);
    
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    device->CreateInputLayout(layoutDesc, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_layout);

    hr = D3DCompile(shaderCode, strlen(shaderCode), nullptr, nullptr, nullptr, "PS", "ps_4_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "PS Error", MB_ICONERROR);
        return false;
    }
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_ps);

    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DYNAMIC;
    vbd.ByteWidth = sizeof(Vertex) * 6;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device->CreateBuffer(&vbd, nullptr, &m_vertexBuffer);

    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(CB);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device->CreateBuffer(&cbd, nullptr, &m_constantBuffer);

    D3D11_SAMPLER_DESC sd = {};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    device->CreateSamplerState(&sd, &m_sampler);

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device->CreateBlendState(&blendDesc, &m_blendState);

    D3D11_RASTERIZER_DESC rd = {};
    rd.CullMode = D3D11_CULL_NONE;
    rd.FillMode = D3D11_FILL_SOLID;
    device->CreateRasterizerState(&rd, &m_rasterState);

    D3D11_DEPTH_STENCIL_DESC dsd = {};
    dsd.DepthEnable = FALSE;
    dsd.StencilEnable = FALSE;
    device->CreateDepthStencilState(&dsd, &m_depthStencilState);

    return true;
}

void SpriteBatch::Begin(ID3D11DeviceContext* context, int screenWidth, int screenHeight) {
    if (screenWidth <= 0 || screenHeight <= 0) return;

    D3D11_VIEWPORT vp = { 0.0f, 0.0f, (float)screenWidth, (float)screenHeight, 0.0f, 1.0f };
    context->RSSetViewports(1, &vp);

    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vs.Get(), nullptr, 0);
    context->PSSetShader(m_ps.Get(), nullptr, 0);
    context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());
    context->OMSetBlendState(m_blendState.Get(), nullptr, 0xFFFFFFFF);
    context->RSSetState(m_rasterState.Get());
    context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

    CB cb;
    // Use row_major in shader + VS mul(v, M) to match XMMatrixOrthographicOffCenterLH directly
    cb.projection = XMMatrixOrthographicOffCenterLH(0, (float)screenWidth, (float)screenHeight, 0, 0, 1);
    
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        memcpy(mapped.pData, &cb, sizeof(cb));
        context->Unmap(m_constantBuffer.Get(), 0);
    }
    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
}

void SpriteBatch::Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv, float x, float y, float w, float h, float srcX, float srcY, float srcW, float srcH) {
    if (!srv) return;

    ComPtr<ID3D11Resource> res;
    srv->GetResource(&res);
    ComPtr<ID3D11Texture2D> tex;
    if (FAILED(res.As(&tex))) return;

    D3D11_TEXTURE2D_DESC desc;
    tex->GetDesc(&desc);

    float u1 = srcX / (float)desc.Width;
    float v1 = srcY / (float)desc.Height;
    float u2 = (srcX + srcW) / (float)desc.Width;
    float v2 = (srcY + srcH) / (float)desc.Height;

    Vertex v[6] = {
        { {x, y + h}, {u1, v2} },
        { {x, y}, {u1, v1} },
        { {x + w, y}, {u2, v1} },
        { {x, y + h}, {u1, v2} },
        { {x + w, y}, {u2, v1} },
        { {x + w, y + h}, {u2, v2} }
    };

    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        memcpy(mapped.pData, v, sizeof(v));
        context->Unmap(m_vertexBuffer.Get(), 0);
    }

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->PSSetShaderResources(0, 1, &srv);
    context->Draw(6, 0);
}

void SpriteBatch::End() {}


#pragma once
#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class D3DContext {
public:
    D3DContext();
    ~D3DContext();

    bool Initialize(HWND hWnd, int width, int height);
    bool Resize(int width, int height); // 添加这一行
    void Clear(float r, float g, float b, float a);
    void Present();
    bool InitImGui(HWND hWnd);
    void CleanupImGui();

    ID3D11Device* GetDevice() const { return m_pd3dDevice.Get(); }
    ID3D11DeviceContext* GetDeviceContext() const { return m_pd3dImmediateContext.Get(); }

private:
    ComPtr<ID3D11Device> m_pd3dDevice;
    ComPtr<ID3D11DeviceContext> m_pd3dImmediateContext;
    ComPtr<IDXGISwapChain> m_pSwapChain;
    ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
};

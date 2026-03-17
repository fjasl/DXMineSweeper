#include "TextureLoader.h"
// Use GDI (LoadImage + GetDIBits) instead of WIC to load BMP files.
// GDI natively handles all BMP formats (1bpp, 4bpp, 8bpp palette, 24bpp, 32bpp)
// without the palette-source complexity of WIC's IWICFormatConverter.
#include <windows.h>
#include <vector>

Texture TextureLoader::LoadTexture(ID3D11Device* device, const std::wstring& filePath) {
    Texture tex = { nullptr, 0, 0 };

    // Load BMP into a GDI HBITMAP. LR_CREATEDIBSECTION keeps it as a DIB
    // (device-independent bitmap), which allows GetDIBits to read raw pixel data.
    HBITMAP hBmp = (HBITMAP)LoadImageW(
        nullptr, filePath.c_str(), IMAGE_BITMAP, 0, 0,
        LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (!hBmp) return tex;

    BITMAP bm = {};
    GetObject(hBmp, sizeof(bm), &bm);
    tex.width  = bm.bmWidth;
    tex.height = bm.bmHeight;

    // Ask GDI to convert to 32bpp BGR (BI_RGB) top-down (negative biHeight).
    // GDI handles 4bpp/8bpp palette expansion automatically.
    BITMAPINFOHEADER bi = {};
    bi.biSize        = sizeof(BITMAPINFOHEADER);
    bi.biWidth       =  bm.bmWidth;
    bi.biHeight      = -bm.bmHeight;  // negative = top-down row order
    bi.biPlanes      = 1;
    bi.biBitCount    = 32;
    bi.biCompression = BI_RGB;

    std::vector<BYTE> buffer(bm.bmWidth * bm.bmHeight * 4, 0);

    HDC hdc = GetDC(nullptr);
    int rows = GetDIBits(hdc, hBmp, 0, (UINT)bm.bmHeight,
                         buffer.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    ReleaseDC(nullptr, hdc);
    DeleteObject(hBmp);

    if (rows == 0) return tex;

    // GetDIBits with BI_RGB leaves the alpha byte as 0 (the "reserved" X byte
    // in BGRX). Set alpha = 255 so all pixels are fully opaque.
    for (int i = 3; i < (int)buffer.size(); i += 4)
        buffer[i] = 255;

    // Upload to D3D11. DXGI_FORMAT_B8G8R8A8_UNORM matches GDI's BGRA byte order.
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width            = (UINT)bm.bmWidth;
    desc.Height           = (UINT)bm.bmHeight;
    desc.MipLevels        = 1;
    desc.ArraySize        = 1;
    desc.Format           = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage            = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA subData = {};
    subData.pSysMem     = buffer.data();
    subData.SysMemPitch = (UINT)bm.bmWidth * 4;

    ComPtr<ID3D11Texture2D> texture;
    HRESULT hr = device->CreateTexture2D(&desc, &subData, &texture);
    if (FAILED(hr)) return tex;

    hr = device->CreateShaderResourceView(texture.Get(), nullptr, &tex.srv);
    if (FAILED(hr)) return tex;

    return tex;
}

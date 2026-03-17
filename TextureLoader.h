#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;

struct Texture {
    ComPtr<ID3D11ShaderResourceView> srv;
    int width;
    int height;
};

class TextureLoader {
public:
    static Texture LoadTexture(ID3D11Device* device, const std::wstring& filePath);
};

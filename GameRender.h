#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "SpriteBatch.h"
#include "TextureLoader.h"
#include "MinesweeperLogic.h"
#include "UIConstants.h"


class GameRenderer {
public:
    bool Initialize(ID3D11Device* device);
    // 主渲染函数，传入上下文、窗口句柄、游戏逻辑对象以及当前的宽高
    void Render(ID3D11DeviceContext* context, HWND hWnd, MinesweeperLogic& logic, int width, int height, int boardW, int boardH);

private:
    void Draw3DLine(ID3D11DeviceContext* ctx, float x, float y, float w, float h, bool light);
    void DrawBaseFrame(ID3D11DeviceContext* ctx, int width, int height, int boardW, int boardH);
    void DrawDigit(ID3D11DeviceContext* context, int value, int x, int y);
    void DrawNumber(ID3D11DeviceContext* context, int num, int x, int y);

    SpriteBatch m_Sprites;
    Texture m_TexBlocks, m_TexLed, m_TexButton;
};

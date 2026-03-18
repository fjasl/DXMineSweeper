#pragma once

enum class SoundEvent {
    Tick,    // 插旗 / 取消插旗
    Explode, // 踩雷
    Win      // 获胜
};

// 播放游戏音效；若 g_bSound 为 false 则静默
void PlayGameSound(SoundEvent ev);

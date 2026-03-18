#pragma comment(lib, "winmm.lib")
#include "SoundManager.h"
#include "AppMenu.h"       // g_bSound
#include "resource1.h"     // IDR_SOUND_*
#include <windows.h>
#include <mmsystem.h>

void PlayGameSound(SoundEvent ev) {
    if (!g_bSound) return;

    UINT id = 0;
    switch (ev) {
    case SoundEvent::Tick:    id = IDR_SOUND_TICK;    break;
    case SoundEvent::Explode: id = IDR_SOUND_EXPLODE; break;
    case SoundEvent::Win:     id = IDR_SOUND_WIN;     break;
    }

    PlaySound(
        MAKEINTRESOURCE(id),
        GetModuleHandle(NULL),
        SND_RESOURCE | SND_ASYNC | SND_NODEFAULT
    );
}

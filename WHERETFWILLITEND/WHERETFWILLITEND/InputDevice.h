#pragma once
#include <windows.h>

class InputDevice
{
private:
    bool m_keys[256] = {};
    POINT m_mouseDelta = { 0,0 };
public:
    void Initialize(HWND hwnd);
    void Update();
    bool IsKeyDown(int vkey) const {
        return m_keys[vkey]; 
    }
    POINT GetMouseDelta() const {
        return m_mouseDelta; 
    }
    void OnRawInput(LPARAM lParam);
    void SetKeyDown(int key, bool down) {
        m_keys[key] = down; 
    }
};

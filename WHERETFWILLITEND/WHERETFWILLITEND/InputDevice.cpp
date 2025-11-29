#include "InputDevice.h"
#include <vector>

void InputDevice::Initialize(HWND hwnd){
    RAWINPUTDEVICE rid[2];
    // мышь
    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x02;
    rid[0].dwFlags = RIDEV_INPUTSINK;
    rid[0].hwndTarget = hwnd;
    // клава
    rid[1].usUsagePage = 0x01;
    rid[1].usUsage = 0x06;
    rid[1].dwFlags = RIDEV_INPUTSINK;
    rid[1].hwndTarget = hwnd;
    RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE));
}
void InputDevice::Update(){
    m_mouseDelta = { 0,0 };
}

void InputDevice::OnRawInput(LPARAM lParam) {
    UINT size = 0;
    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
    if (size == 0) {
        return;
    }
    std::vector<BYTE> buffer(size);
    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer.data(), &size, sizeof(RAWINPUTHEADER)) != size){
        return;
    }
    RAWINPUT* raw = (RAWINPUT*)buffer.data();
    if (raw->header.dwType == RIM_TYPEMOUSE)
    {
        m_mouseDelta.x += raw->data.mouse.lLastX;
        m_mouseDelta.y += raw->data.mouse.lLastY;
    }
    else if (raw->header.dwType == RIM_TYPEKEYBOARD)
    {
        bool pressed = !(raw->data.keyboard.Flags & RI_KEY_BREAK);
        int key = raw->data.keyboard.VKey;
        if (key < 256){
            m_keys[key] = pressed;
        }
    }
}

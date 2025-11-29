#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Window.h"
#include "InputDevice.h"

Window g_Window;
InputDevice g_Input;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow){
    if (!g_Window.Create(hInstance, nCmdShow))
        return 0;
    g_Input.Initialize(g_Window.GetHWND());
    MSG msg = {};
    bool running = true;
    while (running){
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT){
                running = false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        g_Input.Update();
        if (g_Input.IsKeyDown(VK_ESCAPE)){
            PostQuitMessage(0);
        }
    }
    return static_cast<int>(msg.wParam);
}
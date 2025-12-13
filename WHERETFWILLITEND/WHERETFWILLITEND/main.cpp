#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Window.h"
#include "InputDevice.h"
#include "Renderer.h"
#include "Model.h"
#include "Camera.h"
Window g_Window;
InputDevice g_Input;
const int width = 800;
const int height = 800;
const int depth = 1000;
int texture_width = 800;
int texture_height = 800;
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
//light stuff
const float intensity = 5.0;
//Vec3f light_coords = Vec3f(2.0, 0.0, 0.0);
//camera stuff
// Vec3f cam_coords = Vec3f(1.0, 1.0, 1.0);
//const Vec3f look_at = Vec3f(0.0, 0.0, 0.0);
//const Vec3f up = Vec3f(0.0, 1.0, 0.0);
// material stuff
const float ambient_k = 0.1;
const float diffuse_k = 0.5;
const float specular_k = 0.5;
const float shiny_k = 0.8;
int Run() {
    MSG msg = {};
    bool running = true;
    while (running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        g_Input.Update();
        if (g_Input.IsKeyDown(VK_ESCAPE)) {
            PostQuitMessage(0);
        }
        else if (g_Input.IsKeyDown(VK_SPACE)) {
            MessageBox(nullptr, L"Space detected!", L"Input Test", MB_OK);
        }
    }
    return (int)msg.wParam;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow){
    //init window and check if it actually worked
    if (!g_Window.Create(hInstance, nCmdShow))
        return 0;
    //init input device
    g_Input.Initialize(g_Window.GetHWND());
    //catch messege stuff
    int messege = Run();
    return static_cast<int>(messege);
}
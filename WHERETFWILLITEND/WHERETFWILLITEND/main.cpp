#include "Window.h"
#include "InputDevice.h"
#include "NEWRenderer.h"
#include "Model.h"
#include <chrono>
Window g_Window;
InputDevice g_Input;
std::shared_ptr<NewRenderer> g_Renderer;
const int width = 800;
const int height = 800;
const int depth = 1000;
int texture_width = 800;
int texture_height = 800;
//light stuff
const float intensity = 0.1;
XMFLOAT3 light_coords = {0.0, 10.0, 10.0};
//camera stuff
const XMVECTOR cam_coords = {0.0, 2.0, -5.0, 1.0};
const XMVECTOR look_at = {5.0, 5.0, 0.0, 1.0};
const XMVECTOR up = {0.0, 1.0, 0.0, 1.0 };
// material stuff
const float ambient_k = 0.3;
const float diffuse_k = 0.5;
const float specular_k = 0.8;
const float shiny_k = 32;
int Run() {
    MSG msg = {};
    bool running = true;
    using clock = std::chrono::high_resolution_clock;
    auto lastTime = clock::now();
    float time = 0;
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
       auto currentTime = clock::now();
       float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
       lastTime = currentTime;
       time += deltaTime;
       time = std::fmod(time, 4.0f);
       g_Renderer->RenderFrame();
    }
    return (int)msg.wParam;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow){
    //init window and check if it actually worked
    if (!g_Window.Create(hInstance, nCmdShow)){
            return 0;
    }
    //init input device
    g_Input.Initialize(g_Window.GetHWND());
    //catch messege stuff
    g_Renderer = std::make_shared<NewRenderer>(width,height,2, &(g_Window),"sponza.obj", cam_coords, look_at, up, 0);

    int messege = Run();
    return static_cast<int>(messege);
}
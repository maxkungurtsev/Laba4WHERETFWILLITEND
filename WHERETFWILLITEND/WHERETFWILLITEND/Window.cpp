#include "Window.h"
#include "InputDevice.h"

extern InputDevice g_Input;

bool Window::Create(HINSTANCE hInstance, int nCmdShow){
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = L"MyDXWindow";
    wc.hIconSm = wc.hIcon;


    if (!RegisterClassEx(&wc))
        return false;

    RECT WindowRect = { 0,0,1280,720 };
    AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE);

    m_hWnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        L"DirectX App",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WindowRect.right - WindowRect.left,
        WindowRect.bottom - WindowRect.top,
        nullptr, 
        nullptr,
        hInstance, 
        nullptr
    );
    if (!m_hWnd){
        return false;
    }
    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INPUT:
        g_Input.OnRawInput(lParam);
        return 0;
    case WM_KEYDOWN:
        g_Input.SetKeyDown((int)wParam, true);
        return 0;
    case WM_KEYUP:
        g_Input.SetKeyDown((int)wParam, false);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

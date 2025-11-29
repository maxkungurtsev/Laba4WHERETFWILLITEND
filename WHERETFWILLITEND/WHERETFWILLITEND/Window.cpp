#include "Window.h"
#include "InputDevice.h"

extern InputDevice g_Input;

bool Window::Create(HINSTANCE hInstance, int nCmdShow){
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"MyDXWindow";

    if (!RegisterClassEx(&wc))
        return false;

    RECT rect = { 0,0,1280,720 };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    m_hWnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        L"DirectX App",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr, nullptr,
        hInstance, nullptr
    );
    if (!m_hWnd){
        return false;
    }
    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
    return true;
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

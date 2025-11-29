#pragma once
#include <windows.h>

class Window
{
private:
    HWND m_hWnd = nullptr;
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
public:
    bool Create(HINSTANCE hInstance, int nCmdShow);
    HWND GetHWND() const { return m_hWnd; }

};

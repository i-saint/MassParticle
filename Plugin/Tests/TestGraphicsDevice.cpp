#define _CRT_SECURE_NO_WARNINGS
#include <d3d9.h>
#include <d3d12.h>
#include <GL/GL.h>
#include "TestGraphicsDevice.h"
#include "../GraphicsDevice/GraphicsDevice.h"


void TestImpl::testMain()
{

}




LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        // Save the DXSample* passed in to CreateWindow.
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
    }
    return 0;

    case WM_SHOWWINDOW:
    {
        auto* test = (TestImpl*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        test->testMain();
    }
    return 0;

    case WM_KEYDOWN:
        return 0;

    case WM_KEYUP:
        return 0;

    case WM_PAINT:
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void RunTest(TestType type)
{
    TestImpl *test = nullptr;
    switch (type) {
    case TestType::D3D9: test = CreateTestD3D9(); break;
    case TestType::D3D11: test = CreateTestD3D11(); break;
    case TestType::D3D12: test = CreateTestD3D12(); break;
    case TestType::OpenGL: test = CreateTestOpenGL(); break;
    case TestType::Vulkan: test = CreateTestVulkan(); break;
    }

    if (!test) {
        printf("failed to create test\n");
        return;
    }

    // Initialize the window class.
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = GetModuleHandleA(nullptr);
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"TestGraphicsDevice";
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, WindowWidth, WindowHeight };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    auto hwnd = CreateWindow(
        windowClass.lpszClassName,
        L"TestGraphicsDevice",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,		// We have no parent window.
        nullptr,		// We aren't using menus.
        windowClass.hInstance,
        test);

    test->onInit(hwnd);
    ShowWindow(hwnd, SW_SHOWDEFAULT);

    // Main sample loop.
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    delete test;
}

int main(int argc, char *argv[])
{
    printf(
        "choose test:\n"
        "0: D3D9:\n"
        "1: D3D11:\n"
        "2: D3D12:\n"
        "3: OpenGL:\n"
        "4: Vulkan:\n"
    );

    int type;
    scanf("%d", &type);
    RunTest((TestType)type);

    ::Sleep(1000);
    return 0;
}

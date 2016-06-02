#define _CRT_SECURE_NO_WARNINGS
#include <d3d9.h>
#include <d3d12.h>
#include <GL/GL.h>
#include "TestGraphicsDevice.h"
#include "../GraphicsDevice/GraphicsDevice.h"

struct float4 { float x, y, z, w; };
static inline bool operator==(const float4& a, const float4& b) { return memcmp(&a, &b, sizeof(float4)) == 0; }

const char* TranslateResult(gd::Error v) {
    return v == gd::Error::OK ? "ok" : "ng";
}
const char* TranslateResult(bool v) {
    return v ? "ok" : "ng";
}


template<class R>
void PrintResult(R r, int line, const char *exp)
{
    auto t = TranslateResult(r);

#ifdef _WIN32
    // set text color
    auto console = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(console, t[0] == 'o' ? FOREGROUND_GREEN : FOREGROUND_RED);
#endif
    printf("%s %d: %s\n", t, line, exp);
#ifdef _WIN32
    // reset text color
    SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
}

#define Test(exp) PrintResult(exp, __LINE__, #exp)


void TestImpl::testMain()
{
    gd::GraphicsDevice *dev = nullptr;
    switch (getType()) {
    case TestType::D3D9: dev = gd::CreateGraphicsDevice(gd::DeviceType::D3D9, getDevice()); break;
    case TestType::D3D11: dev = gd::CreateGraphicsDevice(gd::DeviceType::D3D11, getDevice()); break;
    case TestType::D3D12: dev = gd::CreateGraphicsDevice(gd::DeviceType::D3D12, getDevice()); break;
    case TestType::OpenGL: dev = gd::CreateGraphicsDevice(gd::DeviceType::OpenGL, getDevice()); break;
    case TestType::Vulkan: dev = gd::CreateGraphicsDevice(gd::DeviceType::Vulkan, getDevice()); break;
    }
    if (!dev) {
        printf("TestImpl::testMain(): device is null\n");
        return;
    }

    // texture read / write texts
    printf("texture read / write texts\n");
    {
        const int width = 1024;
        const int height = 1024;
        const int num_texels = width * height;
        const int data_size = width * height * sizeof(float4);
        gd::TextureFormat format = gd::TextureFormat::RGBAf32;

        std::vector<float4> data;
        data.resize(num_texels);

        for (size_t i = 0; i < data.size(); ++i) {
            float f = (float)i;
            data[i] = { f + 0.0f, f + 0.2f, f + 0.4f, f + 0.8f };
        }

        {
            void *texture = nullptr;
            std::vector<float4> ret(num_texels);
            Test(dev->createTexture(&texture, width, height, format, nullptr, gd::ResourceFlags::None));
            Test(dev->writeTexture(texture, width, height, format, data.data(), data_size));
            Test(dev->readTexture(ret.data(), data_size, texture, width, height, format));
            Test(data.back() == ret.back());
            dev->releaseTexture(texture);
        }
        printf("\n");
        {
            void *rtexture = nullptr;
            std::vector<float4> ret(num_texels);
            Test(dev->createTexture(&rtexture, width, height, format, nullptr, gd::ResourceFlags::CPU_Read));
            Test(dev->writeTexture(rtexture, width, height, format, data.data(), data_size));
            Test(dev->readTexture(ret.data(), data_size, rtexture, width, height, format));
            Test(data.back() == ret.back());
            dev->releaseTexture(rtexture);
        }
        printf("\n");
        {
            void *wtexture = nullptr;
            std::vector<float4> ret(num_texels);
            Test(dev->createTexture(&wtexture, width, height, format, nullptr, gd::ResourceFlags::CPU_Write));
            Test(dev->writeTexture(wtexture, width, height, format, data.data(), data_size));
            Test(dev->readTexture(ret.data(), data_size, wtexture, width, height, format));
            Test(data.back() == ret.back());
            dev->releaseTexture(wtexture);
        }
        printf("\n");
        {
            void *rwtexture = nullptr;
            std::vector<float4> ret(num_texels);
            Test(dev->createTexture(&rwtexture, width, height, format, nullptr, gd::ResourceFlags::CPU_ReadWrite));
            Test(dev->writeTexture(rwtexture, width, height, format, data.data(), data_size));
            Test(dev->readTexture(ret.data(), data_size, rwtexture, width, height, format));
            Test(data.back() == ret.back());
            dev->releaseTexture(rwtexture);
        }
    }

    printf("\n");

    // buffer read / write tests
    printf("buffer read / write tests\n");
    {
        const int num_elements = 1024;
        const int data_size = num_elements * sizeof(float4);
        auto format = gd::BufferType::Vertex;

        std::vector<float4> data;
        data.resize(num_elements);

        for (size_t i = 0; i < data.size(); ++i) {
            float f = (float)i;
            data[i] = { f + 0.0f, f + 0.2f, f + 0.4f, f + 0.8f };
        }

        {
            void *buffer = nullptr;
            std::vector<float4> ret(num_elements);
            Test(dev->createBuffer(&buffer, data_size, format, nullptr, gd::ResourceFlags::None));
            Test(dev->writeBuffer(buffer, data.data(), data_size, format));
            Test(dev->readBuffer(ret.data(), buffer, data_size, format));
            Test(data.back() == ret.back());
            dev->releaseBuffer(buffer);
        }
        printf("\n");
        {
            void *rbuffer = nullptr;
            std::vector<float4> ret(num_elements);
            Test(dev->createBuffer(&rbuffer, data_size, format, nullptr, gd::ResourceFlags::CPU_Read));
            Test(dev->writeBuffer(rbuffer, data.data(), data_size, format));
            Test(dev->readBuffer(ret.data(), rbuffer, data_size, format));
            Test(data.back() == ret.back());
            dev->releaseBuffer(rbuffer);
        }
        printf("\n");
        {
            void *wbuffer = nullptr;
            std::vector<float4> ret(num_elements);
            Test(dev->createBuffer(&wbuffer, data_size, format, nullptr, gd::ResourceFlags::CPU_Write));
            Test(dev->writeBuffer(wbuffer, data.data(), data_size, format));
            Test(dev->readBuffer(ret.data(), wbuffer, data_size, format));
            Test(data.back() == ret.back());
            dev->releaseBuffer(wbuffer);
        }
        printf("\n");
        {
            void *rwbuffer = nullptr;
            std::vector<float4> ret(num_elements);
            Test(dev->createBuffer(&rwbuffer, data_size, format, nullptr, gd::ResourceFlags::CPU_ReadWrite));
            Test(dev->writeBuffer(rwbuffer, data.data(), data_size, format));
            Test(dev->readBuffer(ret.data(), rwbuffer, data_size, format));
            Test(data.back() == ret.back());
            dev->releaseBuffer(rwbuffer);

        }
    }
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

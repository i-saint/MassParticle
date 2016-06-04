#define _CRT_SECURE_NO_WARNINGS
#include <d3d9.h>
#include <d3d12.h>
#include <GL/GL.h>
#include "TestGraphicsInterface.h"
#include "../GraphicsInterface/GraphicsInterface.h"

struct unorm8x4 { uint8_t v[4]; };
struct snorm16x4 { int16_t v[4]; };
struct float32x4 { float v[4]; };
struct int32x4 { int v[4]; };

const char* TranslateResult(gi::Result v) {
    return v == gi::Result::OK ? "ok" : "ng";
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


template<class T>
void TextureTest(gi::GraphicsInterface *ifs, const std::vector<T>& test_data, int width, int height, gi::TextureFormat format, gi::ResourceFlags flags)
{
    const int num_texels = width * height;
    const int data_size = width * height * ifs->GetTexelSize(format);

    void *texture = nullptr;
    auto read_data = std::vector<T>(num_texels);

    Test(ifs->createTexture2D(&texture, width, height, format, nullptr, flags));
    Test(ifs->writeTexture2D(texture, width, height, format, test_data.data(), data_size));
    Test(ifs->readTexture2D(read_data.data(), data_size, texture, width, height, format));
    Test(memcmp(test_data.data(), read_data.data(), data_size) == 0);
    ifs->releaseTexture2D(texture);
    printf("\n");
}

template<class T>
void BufferTest(gi::GraphicsInterface *ifs, const std::vector<T>& test_data, gi::BufferType type, gi::ResourceFlags flags)
{
    const size_t data_size = sizeof(T) * test_data.size();

    void *buffer = nullptr;
    auto read_data = std::vector<T>(test_data.size());

    Test(ifs->createBuffer(&buffer, data_size, type, nullptr, gi::ResourceFlags::None));
    Test(ifs->writeBuffer(buffer, test_data.data(), data_size, type));
    Test(ifs->readBuffer(read_data.data(), buffer, data_size, type));
    Test(memcmp(test_data.data(), read_data.data(), data_size) == 0);
    ifs->releaseBuffer(buffer);
    printf("\n");
}

void TestImpl::testMain()
{
    gi::GraphicsInterface *ifs = nullptr;
    switch (getType()) {
    case TestType::D3D9: ifs = gi::CreateGraphicsInterface(gi::DeviceType::D3D9, getDevice()); break;
    case TestType::D3D11: ifs = gi::CreateGraphicsInterface(gi::DeviceType::D3D11, getDevice()); break;
    case TestType::D3D12: ifs = gi::CreateGraphicsInterface(gi::DeviceType::D3D12, getDevice()); break;
    case TestType::OpenGL: ifs = gi::CreateGraphicsInterface(gi::DeviceType::OpenGL, getDevice()); break;
    case TestType::Vulkan: ifs = gi::CreateGraphicsInterface(gi::DeviceType::Vulkan, getDevice()); break;
    }
    if (!ifs) {
        printf("TestImpl::testMain(): interface is null\n");
        return;
    }

    // texture read / write tests
    {
        const int width = 999; // unfriendly size to make mismatch src-picth and dst-pitch
        const int height = 999;
        gi::TextureFormat format = gi::TextureFormat::RGBAf32;

        auto data = std::vector<float32x4>(width * height);
        for (size_t i = 0; i < data.size(); ++i) {
            float f = (float)i;
            data[i] = { f + 0.0f, f + 0.2f, f + 0.4f, f + 0.8f };
        }

        printf("texture test - RGBAf32\n");
        TextureTest(ifs, data, width, height, format, gi::ResourceFlags::None);

        printf("texture test - RGBAf32 CPU_Read\n");
        TextureTest(ifs, data, width, height, format, gi::ResourceFlags::CPU_Read);

        printf("texture test - RGBAf32 CPU_Write\n");
        TextureTest(ifs, data, width, height, format, gi::ResourceFlags::CPU_Write);

        printf("texture test - RGBAf32 CPU_ReadWrite\n");
        TextureTest(ifs, data, width, height, format, gi::ResourceFlags::CPU_ReadWrite);
    }
    {
        const int width = 999;
        const int height = 999;
        gi::TextureFormat format = gi::TextureFormat::RGBAu8;

        auto data = std::vector<unorm8x4>(width * height);
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = { (i + 0) & 0xff, (i + 1) & 0xff,  (i + 2) & 0xff,  (i + 3) & 0xff };
        }

        printf("texture test - RGBAu8\n");
        TextureTest(ifs, data, width, height, format, gi::ResourceFlags::None);

        printf("texture test - RGBAu8 CPU_Read\n");
        TextureTest(ifs, data, width, height, format, gi::ResourceFlags::CPU_Read);

        printf("texture test - RGBAu8 CPU_Write\n");
        TextureTest(ifs, data, width, height, format, gi::ResourceFlags::CPU_Write);

        printf("texture test - RGBAu8 CPU_ReadWrite\n");
        TextureTest(ifs, data, width, height, format, gi::ResourceFlags::CPU_ReadWrite);
    }


    // buffer read / write tests
    {
        const int num_elements = 9999;
        const int data_size = num_elements * sizeof(int);

        std::vector<int> data;
        data.resize(num_elements);

        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = (int)i;
        }

        printf("buffer test - IndexBuffer i32\n");
        BufferTest(ifs, data, gi::BufferType::Index, gi::ResourceFlags::None);

        printf("buffer test - IndexBuffer i32 CPU_Read\n");
        BufferTest(ifs, data, gi::BufferType::Index, gi::ResourceFlags::CPU_Read);

        printf("buffer test - IndexBuffer i32 CPU_Read\n");
        BufferTest(ifs, data, gi::BufferType::Index, gi::ResourceFlags::CPU_Write);

        printf("buffer test - IndexBuffer i32 CPU_ReadWrite\n");
        BufferTest(ifs, data, gi::BufferType::Index, gi::ResourceFlags::CPU_ReadWrite);
    }
    {
        const int num_elements = 9999;
        const int data_size = num_elements * sizeof(float32x4);

        std::vector<float32x4> data;
        data.resize(num_elements);

        for (size_t i = 0; i < data.size(); ++i) {
            float f = (float)i;
            data[i] = { f + 0.0f, f + 0.2f, f + 0.4f, f + 0.8f };
        }

        printf("buffer test - VertexBuffer RGBAf32\n");
        BufferTest(ifs, data, gi::BufferType::Vertex, gi::ResourceFlags::None);

        printf("buffer test - VertexBuffer RGBAf32 CPU_Read\n");
        BufferTest(ifs, data, gi::BufferType::Vertex, gi::ResourceFlags::CPU_Read);

        printf("buffer test - VertexBuffer RGBAf32 CPU_Read\n");
        BufferTest(ifs, data, gi::BufferType::Vertex, gi::ResourceFlags::CPU_Write);

        printf("buffer test - VertexBuffer RGBAf32 CPU_ReadWrite\n");
        BufferTest(ifs, data, gi::BufferType::Vertex, gi::ResourceFlags::CPU_ReadWrite);


        // these tests fail on d3d9

        printf("buffer test - ComputeBuffer RGBAf32\n");
        BufferTest(ifs, data, gi::BufferType::Compute, gi::ResourceFlags::None);

        printf("buffer test - ComputeBuffer RGBAf32 CPU_Read\n");
        BufferTest(ifs, data, gi::BufferType::Compute, gi::ResourceFlags::CPU_Read);

        printf("buffer test - ComputeBuffer RGBAf32 CPU_Read\n");
        BufferTest(ifs, data, gi::BufferType::Compute, gi::ResourceFlags::CPU_Write);

        printf("buffer test - ComputeBuffer RGBAf32 CPU_ReadWrite\n");
        BufferTest(ifs, data, gi::BufferType::Compute, gi::ResourceFlags::CPU_ReadWrite);
    }
    {
        const int num_elements = 4096;
        const int data_size = num_elements * sizeof(float32x4);

        std::vector<float32x4> data;
        data.resize(num_elements);

        for (size_t i = 0; i < data.size(); ++i) {
            float f = (float)i;
            data[i] = { f + 0.0f, f + 0.2f, f + 0.4f, f + 0.8f };
        }

        // these tests fail on d3d9

        printf("buffer test - ConstantBuffer RGBAf32\n");
        BufferTest(ifs, data, gi::BufferType::Constant, gi::ResourceFlags::None);

        printf("buffer test - ConstantBuffer RGBAf32 CPU_Read\n");
        BufferTest(ifs, data, gi::BufferType::Constant, gi::ResourceFlags::CPU_Read);

        printf("buffer test - ConstantBuffer RGBAf32 CPU_Read\n");
        BufferTest(ifs, data, gi::BufferType::Constant, gi::ResourceFlags::CPU_Write);

        printf("buffer test - ConstantBuffer RGBAf32 CPU_ReadWrite\n");
        BufferTest(ifs, data, gi::BufferType::Constant, gi::ResourceFlags::CPU_ReadWrite);

    }

    gi::ReleaseGraphicsInterface();
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
        for (int i = 0; i < 1; ++i) {
            test->testMain();
        }
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
    windowClass.lpszClassName = L"TestGraphicsInterface";
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, WindowWidth, WindowHeight };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    auto hwnd = CreateWindow(
        windowClass.lpszClassName,
        L"TestGraphicsInterface",
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

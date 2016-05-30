#include "TestGraphicsDevice.h"

#ifdef _WIN32
    #define WithD3D9
    #define WithD3D11
    #define WithD3D12
    #define WithOpenGL
    #define WithVulkan
#else
#endif

#define SafeRelease(v) if(v) { v->Release(); v = nullptr; }




#ifdef WithD3D9
#include <d3d9.h>
#include <dxgi.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dxgi.lib")

class TestImplD3D9 : public TestImpl
{
public:
    ~TestImplD3D9() override;
    TestType getType() const override { return TestType::D3D9; }
    void* getDevice() const override { return m_device; }
    void onInit(void *hwnd) override;

private:
    IDirect3D9 *m_d3d9 = nullptr;
    IDirect3DDevice9 *m_device = nullptr;
};

TestImplD3D9::~TestImplD3D9()
{
    SafeRelease(m_device);
}

void TestImplD3D9::onInit(void *hwnd)
{
    m_d3d9 = ::Direct3DCreate9(D3D_SDK_VERSION);

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.hDeviceWindow = (HWND)hwnd;
    d3dpp.BackBufferCount = 1;
    d3dpp.BackBufferWidth = WindowWidth;
    d3dpp.BackBufferHeight = WindowHeight;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    m_d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, (HWND)hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_device);
}

TestImpl* CreateTestD3D9() { return new TestImplD3D9(); }

#endif // WithD3D9



#ifdef WithD3D11
#include <d3d11.h>
#include <dxgi.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

class TestImplD3D11 : public TestImpl
{
public:
    ~TestImplD3D11() override;
    TestType getType() const override { return TestType::D3D11; }
    void* getDevice() const override { return m_device; }
    void onInit(void *hwnd) override;

private:
    IDXGISwapChain *m_swapchain = nullptr;
    ID3D11Device *m_device = nullptr;
    ID3D11DeviceContext *m_context = nullptr;
};

TestImplD3D11::~TestImplD3D11()
{
    SafeRelease(m_context);
    SafeRelease(m_device);
    SafeRelease(m_swapchain);
}

void TestImplD3D11::onInit(void *hwnd)
{
    D3D_DRIVER_TYPE	dtype = D3D_DRIVER_TYPE_HARDWARE;
    UINT            flags = 0;
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    UINT sdkVersion = D3D11_SDK_VERSION;
    D3D_FEATURE_LEVEL validFeatureLevel;

    DXGI_SWAP_CHAIN_DESC scDesc;
    ZeroMemory(&scDesc, sizeof(scDesc));
    scDesc.BufferCount = 1;
    scDesc.BufferDesc.Width  = WindowWidth;
    scDesc.BufferDesc.Height = WindowHeight;
    scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    scDesc.BufferDesc.RefreshRate.Numerator = 60;
    scDesc.BufferDesc.RefreshRate.Denominator = 1;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.OutputWindow = (HWND)hwnd;
    scDesc.SampleDesc.Count = 1;
    scDesc.SampleDesc.Quality = 0;
    scDesc.Windowed = TRUE;

    IDXGIAdapter *adapter = nullptr;
    HRESULT	hr = D3D11CreateDeviceAndSwapChain(
        adapter,
        dtype,
        nullptr,
        flags,
        featureLevels,
        _countof(featureLevels),
        sdkVersion,
        &scDesc,
        &m_swapchain,
        &m_device,
        &validFeatureLevel,
        &m_context);
}

TestImpl* CreateTestD3D11() { return new TestImplD3D11(); }

#endif // WithD3D11



#ifdef WithD3D12
#include <d3d12.h>
#include <dxgi1_4.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class TestImplD3D12 : public TestImpl
{
public:
    ~TestImplD3D12() override;
    TestType getType() const override { return TestType::D3D12; }
    void* getDevice() const override { return m_device; }
    void onInit(void *hwnd) override;

private:
    ID3D12Device *m_device = nullptr;
    ID3D12CommandQueue *m_queue = nullptr;
    IDXGISwapChain1 *m_swapchain = nullptr;
};

TestImplD3D12::~TestImplD3D12()
{
    SafeRelease(m_swapchain);
    SafeRelease(m_queue);
    SafeRelease(m_device);
}

void TestImplD3D12::onInit(void *hwnd)
{
    IDXGIFactory4 *factory = nullptr;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));

    IDXGIAdapter1 *adapter;
    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            continue;
        }

        if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), (void**)&m_device))) {
            break;
        }
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queue));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = 60;
    swapChainDesc.Width = WindowWidth;
    swapChainDesc.Height = WindowHeight;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    factory->CreateSwapChainForHwnd(
        m_queue,		// Swap chain needs the queue so that it can force a flush on it.
        (HWND)hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &m_swapchain
    );

    // This sample does not support fullscreen transitions.
    factory->MakeWindowAssociation((HWND)hwnd, DXGI_MWA_NO_ALT_ENTER);


    SafeRelease(factory);
}

TestImpl* CreateTestD3D12() { return new TestImplD3D12(); }

#endif // WithD3D12



#ifdef WithOpenGL
#include <GL/GL.h>

class TestImplOpenGL : public TestImpl
{
public:
    ~TestImplOpenGL() override;
    TestType getType() const override { return TestType::OpenGL; }
    void* getDevice() const override { return nullptr; }
    void onInit(void *hwnd) override;

private:
    HWND m_hwnd = nullptr;
    HDC m_hdc = nullptr;
    HGLRC m_hglrc = nullptr;
};

TestImplOpenGL::~TestImplOpenGL()
{
    if (m_hglrc != nullptr) {
        ::wglMakeCurrent(nullptr, nullptr);
        ::wglDeleteContext(m_hglrc);
        m_hglrc = nullptr;
    }
    if (m_hdc != nullptr) {
        ::ReleaseDC(m_hwnd, m_hdc);
        m_hdc = nullptr;
    }
}

void TestImplOpenGL::onInit(void *hwnd)
{
    static PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),    //この構造体のサイズ
        1,                  //OpenGLバージョン
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,       //ダブルバッファ使用可能
        PFD_TYPE_RGBA,      //RGBAカラー
        32,                 //色数
        0, 0,               //RGBAのビットとシフト設定        
        0, 0,                //G
        0, 0,                //B
        0, 0,                //A
        0,                  //アキュムレーションバッファ
        0, 0, 0, 0,         //RGBAアキュムレーションバッファ
        32,                 //Zバッファ    
        0,                  //ステンシルバッファ
        0,                  //使用しない
        PFD_MAIN_PLANE,     //レイヤータイプ
        0,                  //予約
        0, 0, 0             //レイヤーマスクの設定・未使用
    };

    m_hwnd = (HWND)hwnd;
    m_hdc = ::GetDC(m_hwnd);
    int pixelformat = ::ChoosePixelFormat(m_hdc, &pfd);
    ::SetPixelFormat(m_hdc, pixelformat, &pfd);
    m_hglrc = ::wglCreateContext(m_hdc);
    ::wglMakeCurrent(m_hdc, m_hglrc);
}

TestImpl* CreateTestOpenGL() { return new TestImplOpenGL(); }

#endif // WithOpenGL



#ifdef WithVulkan

class TestImplVulkan : public TestImpl
{
public:
    ~TestImplVulkan() override;
    TestType getType() const override { return TestType::Vulkan; }
    void* getDevice() const override { return nullptr; }
    void onInit(void *hwnd) override;

private:
};

TestImplVulkan::~TestImplVulkan()
{
}

void TestImplVulkan::onInit(void *hwnd)
{
}

TestImpl* CreateTestVulkan() { return new TestImplVulkan(); }

#endif // WithVulkan

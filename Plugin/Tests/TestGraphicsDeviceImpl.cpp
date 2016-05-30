#include "TestGraphicsDevice.h"

#ifdef _WIN32
    #define WithD3D9
    #define WithD3D11
    #define WithD3D12
    #define WithOpenGL
    #define WithVulkan
#else
    #define WithOpenGL
#endif

#define SafeRelease(v) if(v) { v->Release(); v = nullptr; }




#ifdef WithD3D9
#include <d3d9.h>
#include <dxgi.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dxgi.lib")

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
    TestType getType() const override;
    void* getDevice() const override;
    void onInit() override;

private:
    IDXGIFactory *m_factory = nullptr;
    ID3D11Device *m_device = nullptr;
    ID3D11DeviceContext *m_context = nullptr;
};

TestImpl* CreateTestD3D11() { return new TestImplD3D11(); }

TestImplD3D11::~TestImplD3D11()
{
    SafeRelease(m_context);
    SafeRelease(m_device);
    SafeRelease(m_factory);
}

TestType TestImplD3D11::getType() const { return TestType::D3D11; }
void* TestImplD3D11::getDevice() const { return m_device; }

void TestImplD3D11::onInit()
{
    HRESULT hr;
    CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&m_factory));

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    UINT deviceCreationFlag = 0;

    hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        deviceCreationFlag,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        &m_device,
        &featureLevel,
        &m_context
    );
}
#endif // WithD3D11



#ifdef WithD3D12
#include <d3d12.h>
#include <dxgi.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#endif // WithD3D12



#ifdef WithOpenGL
#include <GL/GL.h>

#endif // WithOpenGL



#ifdef WithVulkan
#endif // WithVulkan

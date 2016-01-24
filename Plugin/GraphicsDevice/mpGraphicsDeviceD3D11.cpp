#include "pch.h"

#if mpSupportD3D11
#pragma warning( disable : 4996 ) // _s じゃない CRT 関数使うと出るやつ
#pragma warning( disable : 4005 ) // DirectX のマクロの redefinition

#include <d3d11.h>
#include "mpFoundation.h"
#include "mpGraphicsDevice.h"

#define mpSafeRelease(obj) if(obj) { obj->Release(); obj=nullptr; }



class mpGraphicsDeviceD3D11 : public mpGraphicsDevice
{
public:
    mpGraphicsDeviceD3D11(void *dev);
    virtual ~mpGraphicsDeviceD3D11();
    virtual void updateDataTexture(void *tex, int width, int height, const void *data, size_t data_size);

private:
    bool initializeResources();
    HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
    ID3D11Buffer* CreateVertexBuffer(const void *data, UINT size);

private:
    ID3D11Device        *m_pDevice;
    ID3D11DeviceContext *m_pImmediateContext;
};

mpGraphicsDevice* mpCreateGraphicsDeviceD3D11(void *device)
{
    return new mpGraphicsDeviceD3D11(device);
}


mpGraphicsDeviceD3D11::mpGraphicsDeviceD3D11(void *_dev)
: m_pDevice(nullptr)
, m_pImmediateContext(nullptr)
{
    m_pDevice = (ID3D11Device*)_dev;
    m_pDevice->GetImmediateContext(&m_pImmediateContext);
}

mpGraphicsDeviceD3D11::~mpGraphicsDeviceD3D11()
{
    mpSafeRelease(m_pImmediateContext);
}


void mpGraphicsDeviceD3D11::updateDataTexture(void *texptr, int width, int height, const void *data, size_t data_size)
{
    const size_t num_texels = data_size / 16;

    D3D11_BOX box;
    box.left = 0;
    box.right = width;
    box.top = 0;
    box.bottom = ceildiv((UINT)num_texels, (UINT)width);
    box.front = 0;
    box.back = 1;
    ID3D11Texture2D *tex = (ID3D11Texture2D*)texptr;
    m_pImmediateContext->UpdateSubresource(tex, 0, &box, data, width * 16, 0);
}
#endif // mpSupportD3D11

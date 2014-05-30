#pragma warning( disable : 4996 ) // _s じゃない CRT 関数使うと出るやつ
#pragma warning( disable : 4005 ) // DirectX のマクロの redefinition

#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <cstdio>
#include "resource.h"

#include "mpTypes.h"
#include "mpCore_ispc.h"
#include "MassParticle.h"

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT3 Normal;
};

struct CBChangesEveryFrame
{
    XMMATRIX ViewProjection;
    XMFLOAT4 CameraPos;

    XMFLOAT4 LightPos;
    XMFLOAT4 LightColor;

    XMFLOAT4 MeshColor;
    FLOAT    MeshShininess;
    FLOAT    padding[3];
};


class mpRendererD3D11 : public mpRenderer
{
public:
    mpRendererD3D11(void *dev, mpWorld &world);
    virtual ~mpRendererD3D11();
    virtual void render();

private:
    bool initializeDevice(void *dev);
    void finalizeDevice();
    HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
    ID3D11Buffer* CreateVertexBuffer(const void *data, UINT size);

private:
    ID3D11Device            *g_pd3dDevice;
    ID3D11DeviceContext     *g_pImmediateContext;
    ID3D11DepthStencilState *g_pDepthStencilState;
    ID3D11RasterizerState   *g_pRasterState;
    ID3D11BlendState        *g_pBlendState;

    ID3D11VertexShader      *g_pCubeVertexShader;
    ID3D11InputLayout       *g_pCubeVertexLayout;
    ID3D11PixelShader       *g_pCubePixelShader;
    ID3D11Buffer            *g_pCubeVertexBuffer;
    ID3D11Buffer            *g_pCubeInstanceBuffer;
    ID3D11Buffer            *g_pCubeIndexBuffer;
    ID3D11Buffer            *g_pCBChangesEveryFrame;
    XMFLOAT4                g_MeshColor;

    mpWorld *m_world;
};

mpRenderer* mpCreateRendererD3D11(void *device, mpWorld &world)
{
    return new mpRendererD3D11(device, world);
}



//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT mpRendererD3D11::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}


ID3D11Buffer* mpRendererD3D11::CreateVertexBuffer(const void *data, UINT size)
{
    D3D11_BUFFER_DESC bd;
    ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = size;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = data;

    ID3D11Buffer *buffer;
    HRESULT hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &buffer );
    if( FAILED( hr ) ) {
        return NULL;
    }
    return buffer;
}


mpRendererD3D11::mpRendererD3D11(void *_dev, mpWorld &world)
: g_pd3dDevice(nullptr)
, g_pImmediateContext(nullptr)
, g_pDepthStencilState(nullptr)
, g_pRasterState(nullptr)
, g_pBlendState(nullptr)

, g_pCubeVertexShader(nullptr)
, g_pCubeVertexLayout(nullptr)
, g_pCubePixelShader(nullptr)
, g_pCubeVertexBuffer(nullptr)
, g_pCubeInstanceBuffer(nullptr)
, g_pCubeIndexBuffer(nullptr)
, g_pCBChangesEveryFrame(nullptr)
, g_MeshColor(0.7f, 0.7f, 0.7f, 1.0f)
, m_world(&world)
{
    initializeDevice(_dev);
}

mpRendererD3D11::~mpRendererD3D11()
{
    finalizeDevice();
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
bool mpRendererD3D11::initializeDevice(void *_dev)
{
    ID3D11Device *dev = (ID3D11Device*)_dev;
    HRESULT hr = S_OK;

    g_pd3dDevice = dev;
    g_pd3dDevice->GetImmediateContext(&g_pImmediateContext);

    {
        D3D11_RASTERIZER_DESC rsdesc;
        memset(&rsdesc, 0, sizeof(rsdesc));
        rsdesc.FillMode = D3D11_FILL_SOLID;
        rsdesc.CullMode = D3D11_CULL_BACK;
        rsdesc.DepthClipEnable = TRUE;
        rsdesc.MultisampleEnable = TRUE;
        g_pd3dDevice->CreateRasterizerState(&rsdesc, &g_pRasterState);

        D3D11_DEPTH_STENCIL_DESC dsdesc;
        memset(&dsdesc, 0, sizeof(dsdesc));
        dsdesc.DepthEnable = TRUE;
        dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsdesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        g_pd3dDevice->CreateDepthStencilState(&dsdesc, &g_pDepthStencilState);

        D3D11_BLEND_DESC bdesc;
        memset(&bdesc, 0, sizeof(bdesc));
        bdesc.RenderTarget[0].BlendEnable = FALSE;
        bdesc.RenderTarget[0].RenderTargetWriteMask = 0xF;
        g_pd3dDevice->CreateBlendState(&bdesc, &g_pBlendState);
    }

    {
        // Compile the vertex shader
        ID3DBlob* pVSBlob = NULL;
        hr = CompileShaderFromFile( L"MassParticle.fx", "VS", "vs_4_0", &pVSBlob );
        if( FAILED( hr ) )
        {
            MessageBox( NULL,
                        L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
            return false;
        }

        // Create the vertex shader
        hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pCubeVertexShader );
        if( FAILED( hr ) )
        {    
            pVSBlob->Release();
            return false;
        }

        // Create the input layout
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "INSTANCE_POSITION",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_SCALE",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_COLOR",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        };
        UINT numElements = ARRAYSIZE( layout );

        hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
            pVSBlob->GetBufferSize(), &g_pCubeVertexLayout );
        pVSBlob->Release();
        if (FAILED(hr)) {
            return false;
        }
    }

    {
        // Compile the pixel shader
        ID3DBlob* pPSBlob = NULL;
        hr = CompileShaderFromFile( L"MassParticle.fx", "PS", "ps_4_0", &pPSBlob );
        if( FAILED( hr ) )
        {
            MessageBox( NULL,
                L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
            return false;
        }

        // Create the pixel shader
        hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pCubePixelShader );
        pPSBlob->Release();
        if (FAILED(hr)) {
            return false;
        }
    }

    // Create vertex buffer
    {
        SimpleVertex vertices[] =
        {
            { XMFLOAT3(-1.0f, 1.0f,-1.0f),  XMFLOAT3( 0.0f, 1.0f, 0.0f) },
            { XMFLOAT3( 1.0f, 1.0f,-1.0f),  XMFLOAT3( 0.0f, 1.0f, 0.0f) },
            { XMFLOAT3( 1.0f, 1.0f, 1.0f),  XMFLOAT3( 0.0f, 1.0f, 0.0f) },
            { XMFLOAT3(-1.0f, 1.0f, 1.0f),  XMFLOAT3( 0.0f, 1.0f, 0.0f) },

            { XMFLOAT3(-1.0f,-1.0f,-1.0f),  XMFLOAT3( 0.0f,-1.0f, 0.0f) },
            { XMFLOAT3( 1.0f,-1.0f,-1.0f),  XMFLOAT3( 0.0f,-1.0f, 0.0f) },
            { XMFLOAT3( 1.0f,-1.0f, 1.0f),  XMFLOAT3( 0.0f,-1.0f, 0.0f) },
            { XMFLOAT3(-1.0f,-1.0f, 1.0f),  XMFLOAT3( 0.0f,-1.0f, 0.0f) },

            { XMFLOAT3(-1.0f,-1.0f, 1.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f) },
            { XMFLOAT3(-1.0f,-1.0f,-1.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f) },
            { XMFLOAT3(-1.0f, 1.0f,-1.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f) },
            { XMFLOAT3(-1.0f, 1.0f, 1.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f) },

            { XMFLOAT3( 1.0f,-1.0f, 1.0f),  XMFLOAT3( 1.0f, 0.0f, 0.0f) },
            { XMFLOAT3( 1.0f,-1.0f,-1.0f),  XMFLOAT3( 1.0f, 0.0f, 0.0f) },
            { XMFLOAT3( 1.0f, 1.0f,-1.0f),  XMFLOAT3( 1.0f, 0.0f, 0.0f) },
            { XMFLOAT3( 1.0f, 1.0f, 1.0f),  XMFLOAT3( 1.0f, 0.0f, 0.0f) },

            { XMFLOAT3(-1.0f,-1.0f,-1.0f),  XMFLOAT3( 0.0f, 0.0f,-1.0f ) },
            { XMFLOAT3( 1.0f,-1.0f,-1.0f),  XMFLOAT3( 0.0f, 0.0f,-1.0f ) },
            { XMFLOAT3( 1.0f, 1.0f,-1.0f),  XMFLOAT3( 0.0f, 0.0f,-1.0f ) },
            { XMFLOAT3(-1.0f, 1.0f,-1.0f),  XMFLOAT3( 0.0f, 0.0f,-1.0f ) },

            { XMFLOAT3(-1.0f,-1.0f, 1.0f),  XMFLOAT3( 0.0f, 0.0f, 1.0f ) },
            { XMFLOAT3( 1.0f,-1.0f, 1.0f),  XMFLOAT3( 0.0f, 0.0f, 1.0f ) },
            { XMFLOAT3( 1.0f, 1.0f, 1.0f),  XMFLOAT3( 0.0f, 0.0f, 1.0f ) },
            { XMFLOAT3(-1.0f, 1.0f, 1.0f),  XMFLOAT3( 0.0f, 0.0f, 1.0f ) },
        };
        g_pCubeVertexBuffer = CreateVertexBuffer(vertices, sizeof(SimpleVertex)*ARRAYSIZE(vertices));
    }
    {
        g_pCubeInstanceBuffer = CreateVertexBuffer(m_world->particles, sizeof(mpParticle) * SPH_MAX_PARTICLE_NUM);
    }

    // Create index buffer
    {
        WORD indices[] =
        {
            3,1,0, 2,1,3,
            6,4,5, 7,4,6,
            11,9,8, 10,9,11,
            14,12,13, 15,12,14,
            19,17,16, 18,17,19,
            22,20,21, 23,20,22,
        };

        D3D11_BUFFER_DESC bd;
        ZeroMemory( &bd, sizeof(bd) );
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof( WORD ) * 36;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA InitData;
        ZeroMemory( &InitData, sizeof(InitData) );
        InitData.pSysMem = indices;
        hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pCubeIndexBuffer );
        if (FAILED(hr)) {
            return false;
        }
    }

    // Create the constant buffers
    {
        D3D11_BUFFER_DESC bd;
        ZeroMemory( &bd, sizeof(bd) );
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(CBChangesEveryFrame);
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0;
        hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pCBChangesEveryFrame );
        if (FAILED(hr)) {
            return false;
        }
    }

    mpCamera &camera = m_world->m_camera;
    camera.setProjection(XMConvertToRadians(45.0f), 1.0f, 0.1f, 100.0f);
    camera.setView(XMVectorSet(0.0f, 10.0f, -12.5f, 0.0f), XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

    return true;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void mpRendererD3D11::finalizeDevice()
{
    if( g_pCBChangesEveryFrame ) g_pCBChangesEveryFrame->Release();
    if( g_pCubeInstanceBuffer ) g_pCubeInstanceBuffer->Release();
    if( g_pCubeVertexBuffer ) g_pCubeVertexBuffer->Release();
    if( g_pCubeIndexBuffer ) g_pCubeIndexBuffer->Release();
    if( g_pCubeVertexLayout ) g_pCubeVertexLayout->Release();
    if( g_pCubeVertexShader ) g_pCubeVertexShader->Release();
    if( g_pCubePixelShader ) g_pCubePixelShader->Release();

    if (g_pDepthStencilState) g_pDepthStencilState->Release();
    if (g_pRasterState) g_pRasterState->Release();
    if (g_pBlendState) g_pBlendState->Release();

    if( g_pImmediateContext ) g_pImmediateContext->Release();
}

void mpRendererD3D11::render()
{
    {
        std::unique_lock<std::mutex> lock(m_world->m_mutex);

        mpCamera &camera = m_world->m_camera;
        CBChangesEveryFrame cb;
        XMVECTOR eye = camera.getEye();
        XMMATRIX vp = camera.getViewProjectionMatrix();

        cb.ViewProjection = XMMatrixTranspose(vp);
        cb.CameraPos = (FLOAT*)&eye;
        cb.LightPos = XMFLOAT4(10.0f, 10.0f, -10.0f, 1.0f);
        cb.LightColor = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
        cb.MeshShininess = 200.0f;
        g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);
        g_pImmediateContext->UpdateSubresource(g_pCubeInstanceBuffer, 0, NULL, &m_world->particles, 0, 0);
    }
    {
        ID3D11Buffer *buffers[] = {g_pCubeVertexBuffer, g_pCubeInstanceBuffer};
        UINT strides[] = {sizeof(SimpleVertex), sizeof(mpParticle), };
        UINT offsets[] = {0, 0};
        g_pImmediateContext->IASetVertexBuffers( 0, ARRAYSIZE(buffers), buffers, strides, offsets );
    }
    g_pImmediateContext->IASetInputLayout( g_pCubeVertexLayout );
    g_pImmediateContext->IASetIndexBuffer( g_pCubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    g_pImmediateContext->VSSetShader( g_pCubeVertexShader, NULL, 0 );
    g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pCBChangesEveryFrame );
    g_pImmediateContext->PSSetShader( g_pCubePixelShader, NULL, 0 );
    g_pImmediateContext->PSSetConstantBuffers( 0, 1, &g_pCBChangesEveryFrame );


    ID3D11RasterizerState *rs_old;
    ID3D11DepthStencilState *ds_old;
    UINT ds_stencil_ref;
    g_pImmediateContext->RSGetState(&rs_old);
    g_pImmediateContext->OMGetDepthStencilState(&ds_old, &ds_stencil_ref);

    g_pImmediateContext->OMSetDepthStencilState(g_pDepthStencilState, ds_stencil_ref);
    g_pImmediateContext->OMSetBlendState(g_pBlendState, NULL, 0xFFFFFFFF);
    g_pImmediateContext->RSSetState(g_pRasterState);

    // Render cubes
    g_pImmediateContext->DrawIndexedInstanced( 36, (UINT)m_world->num_active_particles, 0, 0, 0 );

    g_pImmediateContext->OMSetDepthStencilState(ds_old, ds_stencil_ref);
    g_pImmediateContext->RSSetState(rs_old);
    rs_old->Release();
    ds_old->Release();
}

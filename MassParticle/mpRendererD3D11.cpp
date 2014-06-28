#pragma warning( disable : 4996 ) // _s じゃない CRT 関数使うと出るやつ
#pragma warning( disable : 4005 ) // DirectX のマクロの redefinition

#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <cstdio>
#include "resource.h"

#include "mpTypes.h"
#include "mpCore_ispc.h"
#include "MassParticle.h"

#define mpSafeRelease(obj) if(obj) { obj->Release(); obj=nullptr; }


struct mpVertex
{
	vec3 Pos;
	vec3 Normal;
};

struct mpCBCData
{
	mat4 ViewProjection;
	vec4 CameraPos;
	vec4 LightPos;
	vec4 LightColor;
	vec4 MeshColor;
	float ParticleSize;
	float MeshShininess;
	float padding[2];
};


class mpRendererD3D11 : public mpRenderer
{
public:
	mpRendererD3D11(void *dev);
	virtual ~mpRendererD3D11();
	virtual void render(mpWorld &world);
	virtual void reloadShader();
	virtual void updateDataTexture(void *tex, const void *data, size_t data_size);

private:
	bool initializeResources();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	ID3D11Buffer* CreateVertexBuffer(const void *data, UINT size);

private:
	ID3D11Device			*m_pDevice;
	ID3D11DeviceContext		*m_pImmediateContext;
	ID3D11DepthStencilState	*m_pDepthStencilState;
	ID3D11RasterizerState	*m_pRasterState;
	ID3D11BlendState		*m_pBlendState;

	ID3D11VertexShader		*m_pCubeVertexShader;
	ID3D11InputLayout		*m_pCubeVertexLayout;
	ID3D11PixelShader		*m_pCubePixelShader;
	ID3D11Buffer			*m_pCubeVertexBuffer;
	ID3D11Buffer			*m_pCubeInstanceBuffer;
	ID3D11Buffer			*m_pCubeIndexBuffer;
	ID3D11Buffer			*m_pCBChangesEveryFrame;
	vec4					m_MeshColor;

	bool m_resources_initialized;
	int m_num_particles;
	int m_max_particles;
};

mpRenderer* mpCreateRendererD3D11(void *device)
{
	return new mpRendererD3D11(device);
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
	hr = D3DX11CompileFromFile( szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
		dwShaderFlags, 0, nullptr, ppBlobOut, &pErrorBlob, nullptr );
	if( FAILED(hr) )
	{
		if( pErrorBlob != nullptr )
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
	HRESULT hr = m_pDevice->CreateBuffer(&bd, !data ? nullptr : &InitData, &buffer);
	if( FAILED( hr ) ) {
		return nullptr;
	}
	return buffer;
}


mpRendererD3D11::mpRendererD3D11(void *_dev)
: m_pDevice(nullptr)
, m_pImmediateContext(nullptr)
, m_pDepthStencilState(nullptr)
, m_pRasterState(nullptr)
, m_pBlendState(nullptr)

, m_pCubeVertexShader(nullptr)
, m_pCubeVertexLayout(nullptr)
, m_pCubePixelShader(nullptr)
, m_pCubeVertexBuffer(nullptr)
, m_pCubeInstanceBuffer(nullptr)
, m_pCubeIndexBuffer(nullptr)
, m_pCBChangesEveryFrame(nullptr)
, m_MeshColor(0.7f, 0.7f, 0.7f, 1.0f)
, m_resources_initialized(false)
, m_num_particles(0)
, m_max_particles(0)
{
	m_pDevice = (ID3D11Device*)_dev;
	m_pDevice->GetImmediateContext(&m_pImmediateContext);
}

mpRendererD3D11::~mpRendererD3D11()
{
	mpSafeRelease(m_pCBChangesEveryFrame);
	mpSafeRelease(m_pCubeInstanceBuffer);
	mpSafeRelease(m_pCubeVertexBuffer);
	mpSafeRelease(m_pCubeIndexBuffer);
	mpSafeRelease(m_pCubeVertexLayout);
	mpSafeRelease(m_pCubeVertexShader);
	mpSafeRelease(m_pCubePixelShader);

	mpSafeRelease(m_pDepthStencilState);
	mpSafeRelease(m_pRasterState);
	mpSafeRelease(m_pBlendState);

	mpSafeRelease(m_pImmediateContext);
}


bool mpRendererD3D11::initializeResources()
{
	if (m_resources_initialized) { return true; }
	m_resources_initialized = true;

	HRESULT hr;
	{
		D3D11_RASTERIZER_DESC rsdesc;
		memset(&rsdesc, 0, sizeof(rsdesc));
		rsdesc.FillMode = D3D11_FILL_SOLID;
		rsdesc.CullMode = D3D11_CULL_BACK;
		rsdesc.DepthClipEnable = TRUE;
		rsdesc.MultisampleEnable = TRUE;
		m_pDevice->CreateRasterizerState(&rsdesc, &m_pRasterState);

		D3D11_DEPTH_STENCIL_DESC dsdesc;
		memset(&dsdesc, 0, sizeof(dsdesc));
		dsdesc.DepthEnable = TRUE;
		dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsdesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		m_pDevice->CreateDepthStencilState(&dsdesc, &m_pDepthStencilState);

		D3D11_BLEND_DESC bdesc;
		memset(&bdesc, 0, sizeof(bdesc));
		bdesc.RenderTarget[0].BlendEnable = FALSE;
		bdesc.RenderTarget[0].RenderTargetWriteMask = 0xF;
		m_pDevice->CreateBlendState(&bdesc, &m_pBlendState);
	}
	reloadShader();


	// Create vertex buffer
	{
		mpVertex vertices[] =
		{
			{ vec3(-1.0f, 1.0f,-1.0f),  vec3( 0.0f, 1.0f, 0.0f) },
			{ vec3( 1.0f, 1.0f,-1.0f),  vec3( 0.0f, 1.0f, 0.0f) },
			{ vec3( 1.0f, 1.0f, 1.0f),  vec3( 0.0f, 1.0f, 0.0f) },
			{ vec3(-1.0f, 1.0f, 1.0f),  vec3( 0.0f, 1.0f, 0.0f) },

			{ vec3(-1.0f,-1.0f,-1.0f),  vec3( 0.0f,-1.0f, 0.0f) },
			{ vec3( 1.0f,-1.0f,-1.0f),  vec3( 0.0f,-1.0f, 0.0f) },
			{ vec3( 1.0f,-1.0f, 1.0f),  vec3( 0.0f,-1.0f, 0.0f) },
			{ vec3(-1.0f,-1.0f, 1.0f),  vec3( 0.0f,-1.0f, 0.0f) },

			{ vec3(-1.0f,-1.0f, 1.0f),  vec3(-1.0f, 0.0f, 0.0f) },
			{ vec3(-1.0f,-1.0f,-1.0f),  vec3(-1.0f, 0.0f, 0.0f) },
			{ vec3(-1.0f, 1.0f,-1.0f),  vec3(-1.0f, 0.0f, 0.0f) },
			{ vec3(-1.0f, 1.0f, 1.0f),  vec3(-1.0f, 0.0f, 0.0f) },

			{ vec3( 1.0f,-1.0f, 1.0f),  vec3( 1.0f, 0.0f, 0.0f) },
			{ vec3( 1.0f,-1.0f,-1.0f),  vec3( 1.0f, 0.0f, 0.0f) },
			{ vec3( 1.0f, 1.0f,-1.0f),  vec3( 1.0f, 0.0f, 0.0f) },
			{ vec3( 1.0f, 1.0f, 1.0f),  vec3( 1.0f, 0.0f, 0.0f) },

			{ vec3(-1.0f,-1.0f,-1.0f),  vec3( 0.0f, 0.0f,-1.0f ) },
			{ vec3( 1.0f,-1.0f,-1.0f),  vec3( 0.0f, 0.0f,-1.0f ) },
			{ vec3( 1.0f, 1.0f,-1.0f),  vec3( 0.0f, 0.0f,-1.0f ) },
			{ vec3(-1.0f, 1.0f,-1.0f),  vec3( 0.0f, 0.0f,-1.0f ) },

			{ vec3(-1.0f,-1.0f, 1.0f),  vec3( 0.0f, 0.0f, 1.0f ) },
			{ vec3( 1.0f,-1.0f, 1.0f),  vec3( 0.0f, 0.0f, 1.0f ) },
			{ vec3( 1.0f, 1.0f, 1.0f),  vec3( 0.0f, 0.0f, 1.0f ) },
			{ vec3(-1.0f, 1.0f, 1.0f),  vec3( 0.0f, 0.0f, 1.0f ) },
		};
		m_pCubeVertexBuffer = CreateVertexBuffer(vertices, sizeof(mpVertex)*ARRAYSIZE(vertices));
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
		hr = m_pDevice->CreateBuffer( &bd, &InitData, &m_pCubeIndexBuffer );
		if (FAILED(hr)) {
			return false;
		}
	}

	// Create the constant buffers
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(mpCBCData);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		hr = m_pDevice->CreateBuffer( &bd, nullptr, &m_pCBChangesEveryFrame );
		if (FAILED(hr)) {
			return false;
		}
	}

	return true;
}

void mpRendererD3D11::reloadShader()
{
	if (!m_resources_initialized) { return; }

	HRESULT hr;
	ID3D11VertexShader *pCubeVertexShader = nullptr;
	ID3D11InputLayout  *pCubeVertexLayout = nullptr;
	ID3D11PixelShader  *pCubePixelShader = nullptr;

	{
		// Compile the vertex shader
		ID3DBlob* pVSBlob = nullptr;
		hr = CompileShaderFromFile(L"MassParticle.fx", "VS", "vs_4_0", &pVSBlob);
		if (FAILED(hr))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			goto on_failed;
		}

		// Create the vertex shader
		hr = m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pCubeVertexShader);
		if (FAILED(hr))
		{
			pVSBlob->Release();
			goto on_failed;
		}

		// Create the input layout
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "INSTANCE_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INSTANCE_VELOCITY", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INSTANCE_PARAMS",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
		UINT numElements = ARRAYSIZE(layout);

		hr = m_pDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
			pVSBlob->GetBufferSize(), &pCubeVertexLayout);
		pVSBlob->Release();
		if (FAILED(hr)) {
			goto on_failed;
		}
	}

	{
		// Compile the pixel shader
		ID3DBlob* pPSBlob = nullptr;
		hr = CompileShaderFromFile(L"MassParticle.fx", "PS", "ps_4_0", &pPSBlob);
		if (FAILED(hr))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			goto on_failed;
		}

		// Create the pixel shader
		hr = m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &pCubePixelShader);
		pPSBlob->Release();
		if (FAILED(hr)) {
			goto on_failed;
		}
	}

	mpSafeRelease(m_pCubeVertexLayout);
	mpSafeRelease(m_pCubeVertexShader);
	mpSafeRelease(m_pCubePixelShader);
	m_pCubeVertexLayout = pCubeVertexLayout;
	m_pCubeVertexShader = pCubeVertexShader;
	m_pCubePixelShader = pCubePixelShader;
	return;

on_failed:
	mpSafeRelease(pCubeVertexLayout);
	mpSafeRelease(pCubeVertexShader);
	mpSafeRelease(pCubePixelShader);
	return;
}

void mpRendererD3D11::render(mpWorld &world)
{
	initializeResources();
	if (!m_pCubePixelShader || !m_pCubeVertexShader) { return;  }

	const mpKernelParams &kparams = world.getKernelParams();
	if (m_max_particles < kparams.max_particles) {
		m_max_particles = kparams.max_particles;
		mpSafeRelease(m_pCubeInstanceBuffer);
		m_pCubeInstanceBuffer = CreateVertexBuffer(nullptr, sizeof(mpParticle) * m_max_particles);
	}

	int num_particles;
	{
		std::unique_lock<std::mutex> lock(world.getMutex());
		mpParticle *particles = world.getParticlesGPU();
		num_particles = world.getNumParticlesGPU();
		if (num_particles == 0) { return; }
		m_pImmediateContext->UpdateSubresource(m_pCubeInstanceBuffer, 0, nullptr, particles, 0, 0);
	}
	{
		mpCBCData cb;
		vec4 eye(1.0f, 1.0f, 1.0f, 1.0f);
		mat4 vp;
		world.getViewProjection(vp, (vec3&)eye);

		cb.ViewProjection = glm::transpose(vp);
		cb.CameraPos = eye;
		cb.LightPos = vec4(10.0f, 10.0f, -10.0f, 1.0f);
		cb.LightColor = vec4(0.9f, 0.9f, 0.9f, 1.0f);
		cb.MeshShininess = 200.0f;
		cb.ParticleSize = world.getKernelParams().particle_size;
		m_pImmediateContext->UpdateSubresource(m_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0);
	}
	{
		ID3D11Buffer *buffers[] = {m_pCubeVertexBuffer, m_pCubeInstanceBuffer};
		UINT strides[] = {sizeof(mpVertex), sizeof(mpParticle), };
		UINT offsets[] = {0, 0};
		m_pImmediateContext->IASetVertexBuffers( 0, ARRAYSIZE(buffers), buffers, strides, offsets );
	}
	m_pImmediateContext->IASetInputLayout( m_pCubeVertexLayout );
	m_pImmediateContext->IASetIndexBuffer( m_pCubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
	m_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	m_pImmediateContext->VSSetShader( m_pCubeVertexShader, nullptr, 0 );
	m_pImmediateContext->VSSetConstantBuffers( 0, 1, &m_pCBChangesEveryFrame );
	m_pImmediateContext->PSSetShader( m_pCubePixelShader, nullptr, 0 );
	m_pImmediateContext->PSSetConstantBuffers( 0, 1, &m_pCBChangesEveryFrame );


	ID3D11RasterizerState *rs_old;
	ID3D11DepthStencilState *ds_old;
	UINT ds_stencil_ref;
	m_pImmediateContext->RSGetState(&rs_old);
	m_pImmediateContext->OMGetDepthStencilState(&ds_old, &ds_stencil_ref);

	m_pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, ds_stencil_ref);
	m_pImmediateContext->OMSetBlendState(m_pBlendState, nullptr, 0xFFFFFFFF);
	m_pImmediateContext->RSSetState(m_pRasterState);

	// Render cubes
	m_pImmediateContext->DrawIndexedInstanced( 36, (UINT)num_particles, 0, 0, 0 );

	m_pImmediateContext->OMSetDepthStencilState(ds_old, ds_stencil_ref);
	m_pImmediateContext->RSSetState(rs_old);
	rs_old->Release();
	ds_old->Release();
}

void mpRendererD3D11::updateDataTexture(void *texptr, const void *data, size_t data_size)
{
	const int num_texels = data_size / 16;

	D3D11_BOX box;
	box.left = 0;
	box.right = mpDataTextureWidth;
	box.top = 0;
	box.bottom = ceildiv(num_texels, mpDataTextureWidth);
	box.front = 0;
	box.back = 1;
	ID3D11Texture2D *tex = (ID3D11Texture2D*)texptr;
	m_pImmediateContext->UpdateSubresource(tex, 0, &box, data, mpDataTextureWidth * 16, 0);
}

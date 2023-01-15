#include "pch.h"
#include "Renderer.h"
#include "Utils.h"
#include "Sampler.h"
#include "OpaqueEffect.h"
#include "OpaqueMesh.h"
#include "PartialCoverageMesh.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		m_RasterizerDesc.FillMode = D3D11_FILL_SOLID;
		m_RasterizerDesc.CullMode = D3D11_CULL_BACK;

		ID3D11RasterizerState* pRasterizerState{};

		result = m_pDevice->CreateRasterizerState(&m_RasterizerDesc, &pRasterizerState);

		if (FAILED(result))
			assert("Failed to create rasterizerState");
		
		//Create Samplers
		m_pPointSampler = new Sampler(m_pDevice, Sampler::SamplerStateKind::point);
		m_pLinearSampler = new Sampler(m_pDevice, Sampler::SamplerStateKind::linear);
		m_pAnisotropicSampler = new Sampler(m_pDevice, Sampler::SamplerStateKind::anisotropic);

		//create the meshes and change it samplerState
		m_pCombustionEffectMesh = new PartialCoverageMesh(m_pDevice, "Resources/fireFX.obj", L"Resources/PosUV.fx");
		m_pVehicleMesh = new OpaqueMesh(m_pDevice, "Resources/vehicle.obj", L"Resources/PosTex.fx");
		m_pVehicleMesh->ChangeSamplerState(m_pDevice, m_pPointSampler);
		m_pVehicleMesh->SetRasterizerState(pRasterizerState);
		
		//initialize the camera
		m_Camera.Initialize(45, { 0.f, 0.f, -50.f }, m_Width / static_cast<float>(m_Height));

		//create texture
		m_pCombustionEffectDiffuse = Texture::LoadFromFile("Resources/fireFX_diffuse.png", m_pDevice);
		m_pVehicleDiffuse = Texture::LoadFromFile("Resources/vehicle_diffuse.png", m_pDevice);
		m_pNormal = Texture::LoadFromFile("Resources/vehicle_normal.png", m_pDevice);
		m_pSpecular = Texture::LoadFromFile("Resources/vehicle_specular.png", m_pDevice);
		m_pGlossiness = Texture::LoadFromFile("Resources/vehicle_gloss.png", m_pDevice);

		m_pCombustionEffectMesh->SetDiffuseMap(m_pCombustionEffectDiffuse);
		m_pVehicleMesh->SetDiffuseMap(m_pVehicleDiffuse);
		m_pVehicleMesh->SetNormalMap(m_pNormal);
		m_pVehicleMesh->SetSpecularMap(m_pSpecular);
		m_pVehicleMesh->SetGlossinessMap(m_pGlossiness);
	}

	Renderer::~Renderer()
	{
		if (m_pRenderTargetView)
			m_pRenderTargetView->Release();

		if (m_pRenderTargetBuffer)
			m_pRenderTargetBuffer->Release();

		if (m_pDepthStencilView)
			m_pDepthStencilView->Release();

		if (m_pDepthStencilBuffer)
			m_pDepthStencilBuffer->Release();

		if (m_pSwapChain)
			m_pSwapChain->Release();

		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}

		if (m_pDevice)
			m_pDevice->Release();

		delete m_pCombustionEffectMesh;
		delete m_pVehicleMesh;
		
		delete m_pCombustionEffectDiffuse;
		delete m_pVehicleDiffuse;
		delete m_pNormal;
		delete m_pSpecular;
		delete m_pGlossiness;

		delete m_pPointSampler;
		delete m_pLinearSampler;
		delete m_pAnisotropicSampler;
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_Camera.Update(pTimer);

		if (m_IsRotating)
		{
			m_pCombustionEffectMesh->RotateYCW(m_pCombustionEffectMesh->GetRotationAngle() + pTimer->GetElapsed() * m_pCombustionEffectMesh->GetRotationSpeed());
			m_pVehicleMesh->RotateYCW(m_pVehicleMesh->GetRotationAngle() + pTimer->GetElapsed() * m_pVehicleMesh->GetRotationSpeed());
		}

		m_pCombustionEffectMesh->SetWorldViewProjMatrix(m_pCombustionEffectMesh->GetWorldMatrix() * m_Camera.viewMatrix * m_Camera.projectionMatrix);
		m_pVehicleMesh->SetWorldViewProjMatrix(m_pVehicleMesh->GetWorldMatrix() * m_Camera.viewMatrix * m_Camera.projectionMatrix);
		m_pVehicleMesh->SetViewInverseMatrix(m_Camera.invViewMatrix);
	}

	void Renderer::Render() const
	{
		if (!m_IsInitialized)
			return;

		//1. Clear RTV and DSV
		ColorRGB clearColor{ 0.39f, 0.59f, 0.93f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		//2. Set pipeline + invoke drawcalls (= render)
		m_pVehicleMesh->Render(m_pDeviceContext);

		if (m_RenderFireFX)
		{
			m_pCombustionEffectMesh->Render(m_pDeviceContext);
		}
		
		//3. Present BackBuffer (swap)
		m_pSwapChain->Present(0, 0);
	}

	HRESULT Renderer::InitializeDirectX()
	{
		//1. Create Device and DeviceContext
		// Device: represents the display adapter, is used to create resources
		// DeviceContex: contains the setting in wich a device is used,
		// used to set pipeline states and generate rendering commands using the resources owned by the device, NOT threat safe

		D3D_FEATURE_LEVEL featureLevel{ D3D_FEATURE_LEVEL_11_1 };
		uint32_t createDeviceFlags{ 0 };

#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG; // |= assignment by bitwise or
#endif

		HRESULT result
		{ 
			D3D11CreateDevice
			(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				createDeviceFlags,
				&featureLevel,
				1,
				D3D11_SDK_VERSION,
				&m_pDevice,
				nullptr,
				&m_pDeviceContext										
			) 
		};

		if (FAILED(result))
			return result;

		//Create DXGI Factory
		IDXGIFactory* pDxgiFactory{};

		result = CreateDXGIFactory1(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pDxgiFactory));

		if (FAILED(result))
			return result;

		//2. Create SwapChain
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		//Get the handle (HWND) from the SDL Backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);

		pDxgiFactory->Release();

		if (FAILED(result))
			return result;

		//3. Create DepthStencil (DS) and DepthStencilView (DSV)
		// DepthStencil: a buffer used to mask pixels in an image
		// Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		//View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		
		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);

		if (FAILED(result))
			return result;

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);

		if (FAILED(result))
			return result;

		//4. Create RenderTarget (RT) and RenderTargetView (RTV)
		//Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));

		if (FAILED(result))
			return result;

		//View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);

		if (FAILED(result))
			return result;

		//5. Bind RTV and DSV to Output Merger Stage
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//6. Set ViewPort
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		return result;
	}

	void Renderer::ChangeSamplerState()
	{
		switch (m_pVehicleMesh->GetSamplerStateKind())
		{
		case Sampler::SamplerStateKind::point:
			m_pVehicleMesh->ChangeSamplerState(m_pDevice, m_pLinearSampler);
			std::cout << "SamplerState changed to linear\n";
			break;

		case Sampler::SamplerStateKind::linear:
			m_pVehicleMesh->ChangeSamplerState(m_pDevice, m_pAnisotropicSampler);
			std::cout << "SamplerState changed to anisotropic\n";
			break;

		case Sampler::SamplerStateKind::anisotropic:
			m_pVehicleMesh->ChangeSamplerState(m_pDevice, m_pPointSampler);
			std::cout << "SamplerState changed to point\n";
			break;
		}
	}

	void Renderer::CycleCullModes()
	{
		switch (m_RasterizerDesc.CullMode)
		{
		case D3D11_CULL_BACK:
			m_RasterizerDesc.CullMode = D3D11_CULL_FRONT;
			std::cout << "CullMode = FRONT\n";
			break;
		
		case D3D11_CULL_FRONT:
			m_RasterizerDesc.CullMode = D3D11_CULL_NONE;
			std::cout << "CullMode = NONE\n";
			break;

		case D3D11_CULL_NONE:
			m_RasterizerDesc.CullMode = D3D11_CULL_BACK;
			std::cout << "CullMode = BACK\n";
			break;
		}

		HRESULT result{};
		ID3D11RasterizerState* pRasterizerState{};
		result = m_pDevice->CreateRasterizerState(&m_RasterizerDesc, &pRasterizerState);

		if (FAILED(result))
			assert("Failed to change CullMode\n");

		m_pVehicleMesh->SetRasterizerState(pRasterizerState);
	}

	void Renderer::ToggleIsRotating()
	{
		m_IsRotating = !m_IsRotating;
	}

	void Renderer::ToggleFireFX()
	{
		m_RenderFireFX = !m_RenderFireFX;
	}
}


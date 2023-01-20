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
			m_DirectXIsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		InitializeSoftwareRasterizer();

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
		m_pFireFXMesh = new PartialCoverageMesh(m_pDevice, "Resources/fireFX.obj", L"Resources/PosUV.fx", Mesh::CullMode::None, static_cast<float>(m_Width), static_cast<float>(m_Height));
		m_pVehicleMesh = new OpaqueMesh(m_pDevice, "Resources/vehicle.obj", L"Resources/PosTex.fx", Mesh::CullMode::BackFace, static_cast<float>(m_Width), static_cast<float>(m_Height));
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

		m_pFireFXMesh->SetDiffuseMap(m_pCombustionEffectDiffuse);
		m_pVehicleMesh->SetDiffuseMap(m_pVehicleDiffuse);
		m_pVehicleMesh->SetNormalMap(m_pNormal);
		m_pVehicleMesh->SetSpecularMap(m_pSpecular);
		m_pVehicleMesh->SetGlossinessMap(m_pGlossiness);

		PrintInfo();
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

		delete m_pFireFXMesh;
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
			m_pFireFXMesh->RotateYCW(m_pFireFXMesh->GetRotationAngle() + pTimer->GetElapsed() * m_pFireFXMesh->GetRotationSpeed());
			m_pVehicleMesh->RotateYCW(m_pVehicleMesh->GetRotationAngle() + pTimer->GetElapsed() * m_pVehicleMesh->GetRotationSpeed());
		}

		m_pFireFXMesh->SetWorldViewProjMatrix(m_pFireFXMesh->GetWorldMatrix() * m_Camera.viewMatrix * m_Camera.projectionMatrix);
		m_pVehicleMesh->SetWorldViewProjMatrix(m_pVehicleMesh->GetWorldMatrix() * m_Camera.viewMatrix * m_Camera.projectionMatrix);
		m_pVehicleMesh->SetViewInverseMatrix(m_Camera.invViewMatrix);
	}

	void Renderer::Render() const
	{
		switch (m_RenderMode)
		{
		case dae::Renderer::RenderMode::Directx:
			RenderInDirectX();
			break;

		case dae::Renderer::RenderMode::SoftwareRasterizer:
			RenderInSoftwareRasterizer();
			break;
		}
	}

	void Renderer::InitializeSoftwareRasterizer()
	{
		//Create Buffers
		m_pFrontBuffer = SDL_GetWindowSurface(m_pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

		const int amountOfPixels{ m_Width * m_Height };
		m_pDepthBufferPixels = new float[amountOfPixels];
		//fill depth buffer with ifiniy as value
		for (int index{}; index < amountOfPixels; ++index)
		{
			m_pDepthBufferPixels[index] = INFINITY;
		}
	}

	void Renderer::RenderInSoftwareRasterizer() const
	{
		//@START
		if (m_UseUniformClearColor)
		{
			SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, static_cast<Uint8>(m_UniformClearColor.r * 255.f), static_cast<Uint8>(m_UniformClearColor.g * 255.f), static_cast<Uint8>(m_UniformClearColor.b * 255.f)));
		}
		else
		{
			Uint8 redValue{ 99 }, greenValue{ 99 }, blueValue{ 99 };
			SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, redValue, greenValue, blueValue));
		}
		
		std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, INFINITY);
		//Lock BackBuffer
		SDL_LockSurface(m_pBackBuffer);

		m_pVehicleMesh->RenderInSoftwareRasterizer(m_Camera, m_pDepthBufferPixels, m_pBackBuffer, m_pBackBufferPixels);

		if(m_RenderFireFX)
			m_pFireFXMesh->RenderInSoftwareRasterizer(m_Camera, m_pDepthBufferPixels, m_pBackBuffer, m_pBackBufferPixels);

		//@END
	//Update SDL Surface
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
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
		IDXGIFactory1* pDxgiFactory{};

		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));

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

	void Renderer::RenderInDirectX() const
	{
		if (!m_DirectXIsInitialized)
			return;

		//1. Clear RTV and DSV
		if (m_UseUniformClearColor)
		{
			m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &m_UniformClearColor.r);
		}
		else
		{
			ColorRGBA clearColor{ 0.39f, 0.59f, 0.93f };
			m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		}
		
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		//2. Set pipeline + invoke drawcalls (= render)
		m_pVehicleMesh->RenderInDirectX(m_pDeviceContext);

		if (m_RenderFireFX)
			m_pFireFXMesh->RenderInDirectX(m_pDeviceContext);

		//3. Present BackBuffer (swap)
		m_pSwapChain->Present(0, 0);
	}

	void Renderer::ChangeSamplerState()
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 2); //set console text color to green

		switch (m_pVehicleMesh->GetSamplerStateKind())
		{
		case Sampler::SamplerStateKind::point:
			m_pVehicleMesh->ChangeSamplerState(m_pDevice, m_pLinearSampler);
			std::cout << "Sampler Filter = LINEAR\n";
			break;

		case Sampler::SamplerStateKind::linear:
			m_pVehicleMesh->ChangeSamplerState(m_pDevice, m_pAnisotropicSampler);
			std::cout << "Sampler Filter = ANISOTROPIC\n";
			break;

		case Sampler::SamplerStateKind::anisotropic:
			m_pVehicleMesh->ChangeSamplerState(m_pDevice, m_pPointSampler);
			std::cout << "Sampler Filter = POINT\n";
			break;
		}

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); //set console text color to white
	}

	void Renderer::CycleCullModes()
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6); //set console text color to orange

		switch (m_RasterizerDesc.CullMode)
		{
		case D3D11_CULL_BACK:
			m_RasterizerDesc.CullMode = D3D11_CULL_FRONT;
			m_pVehicleMesh->SetCullMode(Mesh::CullMode::FrontFace);
			std::cout << "CullMode = FRONT\n";
			break;
		
		case D3D11_CULL_FRONT:
			m_RasterizerDesc.CullMode = D3D11_CULL_NONE;
			m_pVehicleMesh->SetCullMode(Mesh::CullMode::None);
			std::cout << "CullMode = NONE\n";
			break;

		case D3D11_CULL_NONE:
			m_RasterizerDesc.CullMode = D3D11_CULL_BACK;
			m_pVehicleMesh->SetCullMode(Mesh::CullMode::BackFace);
			std::cout << "CullMode = BACK\n";
			break;
		}

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); //set console text color to white

		HRESULT result{};
		ID3D11RasterizerState* pRasterizerState{};
		result = m_pDevice->CreateRasterizerState(&m_RasterizerDesc, &pRasterizerState);

		if (FAILED(result))
			assert("Failed to change CullMode\n");

		m_pVehicleMesh->SetRasterizerState(pRasterizerState);
	}

	void Renderer::CycleRenderModes()
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6); //set console text color to orange

		switch (m_RenderMode)
		{
		case RenderMode::Directx:
			m_RenderMode = RenderMode::SoftwareRasterizer;
			std::cout << "Rasterizer Mode = SOFTWARE\n";
			break;

		case RenderMode::SoftwareRasterizer:
			m_RenderMode = RenderMode::Directx;
			std::cout << "Rasterizer Mode = HARDWARE\n";
			break;
		}

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); //set console text color to white
	}

	void Renderer::ToggleIsRotating()
	{
		m_IsRotating = !m_IsRotating;

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6); //set console text color to orange

		if (m_IsRotating)
			std::cout << "Rotation On\n";
		else
			std::cout << "Rotation Off\n";

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); //set console text color to white
	}

	void Renderer::ToggleUseUniformClearColor()
	{
		m_UseUniformClearColor = !m_UseUniformClearColor;

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6); //set console text color to orange

		if (m_UseUniformClearColor)
			std::cout << "Uniform ClearColor On\n";
		else
			std::cout << "Uniform ClearColor Off\n";

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); //set console text color to white
	}

	void Renderer::ToggleFireFX()
	{
		m_RenderFireFX = !m_RenderFireFX;

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6); //set console text color to orange

		if(m_RenderFireFX)
			std::cout << "FireFX mesh On\n";
		else
			std::cout << "FireFX mesh Off\n";

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); //set console text color to white
	}

	void Renderer::CycleShadingMode()
	{
		if (m_RenderMode == RenderMode::SoftwareRasterizer)
			m_pVehicleMesh->CycleShadingMode();
	}

	void Renderer::ToggleNormalMap()
	{
		if (m_RenderMode == RenderMode::SoftwareRasterizer)
			m_pVehicleMesh->ToggleUseNormalMap();
	}

	void Renderer::ToggleDepthBufferVisualization()
	{
		if (m_RenderMode == RenderMode::SoftwareRasterizer)
			m_pVehicleMesh->ToggleDepthBufferVisualization();
	}

	void Renderer::ToggleBoundingBoxVisualization()
	{
		if (m_RenderMode == RenderMode::SoftwareRasterizer)
			m_pVehicleMesh->ToggleBoundingBoxVisualization();
	}

	void Renderer::PrintInfo()
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6); //set console text color to orange
		
		std::cout << "KEY BINDINGS:\n";
		std::cout << "\n\tSHARED:\n";
		std::cout << "\tToggle between DirectX & Software Rasterizer [F1]\n";
		std::cout << "\tToggle Rotation (On/Off) [F2]\n";
		std::cout << "\tToggle FireFX mesh (On/Off) [F3]\n";
		std::cout << "\tCycle Cull Modes (back-face, front-face, none) [F9]\n";
		std::cout << "\tToggle Uniform ClearColor [F10]\n";
		std::cout << "\tToggle Print FPS (On/Off) [F11]\n";

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 2); //set console text color to green

		std::cout << "\n\tHARDWARE ONLY:\n";
		std::cout << "\tToggle between Texture Sampling States (point-linear-anisotropic) [F4]\n";

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 5); //set console text color to purple

		std::cout << "\n\tSOFTWARE ONLY:\n";
		std::cout << "\tCycle Shading Mode (Combined/ObservedArea/Diffuse/Specular) [F5]\n";
		std::cout << "\tToggle NormalMap (On/Off) [F6]\n";
		std::cout << "\tToggle DepthBuffer Visualization (On/Off) [F7]\n";
		std::cout << "\tToggle BoundingBox Visualization (On/Off) [F8]\n";

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); //set console text color to white
	}
}


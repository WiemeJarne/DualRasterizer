#pragma once
#include "Camera.h"
#include "Texture.h"

struct SDL_Window;
struct SDL_Surface;
class Sampler;

namespace dae
{
	class Mesh;
	class OpaqueMesh;
	class PartialCoverageMesh;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;
		void ChangeSamplerState();
		void CycleCullModes();
		void ToggleIsRotating();
		void ToggleFireFX();

	private:
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		Camera m_Camera{};

		bool m_IsInitialized{ false };
		bool m_UseUniformClearColor{ false };

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGISwapChain* m_pSwapChain;
		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;
		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;

		//SamplerStates
		Sampler* m_pPointSampler;
		Sampler* m_pLinearSampler;
		Sampler* m_pAnisotropicSampler;

		//Textures
		Texture* m_pCombustionEffectDiffuse;
		Texture* m_pVehicleDiffuse;
		Texture* m_pNormal;
		Texture* m_pSpecular;
		Texture* m_pGlossiness;

		OpaqueMesh* m_pVehicleMesh{};
		PartialCoverageMesh* m_pCombustionEffectMesh{};

		D3D11_RASTERIZER_DESC m_RasterizerDesc{};

		bool m_IsRotating{ true };
		bool m_RenderFireFX{ true };

		//DIRECTX
		HRESULT InitializeDirectX();
		//...
	};
}

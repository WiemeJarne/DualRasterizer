#include "pch.h"
#include "Texture.h"
#include "Vector2.h"
#include <cassert>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface, ID3D11Device* pDevice)
		: m_pSurface{ pSurface }
		, m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
		DXGI_FORMAT format{ DXGI_FORMAT_R8G8B8A8_UNORM };
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = pSurface->w;
		desc.Height = pSurface->h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = pSurface->pixels;
		initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
		initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

		HRESULT result{ pDevice->CreateTexture2D(&desc, &initData, &m_pResource) };

		if (FAILED(result))
			assert("Failed to create directX resource");

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		result = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pSRV);

		if (FAILED(result))
			assert("Failed to create directX resource view");
	}

	Texture::~Texture()
	{
		if(m_pResource)
			m_pResource->Release();

		if(m_pSRV)
			m_pSRV->Release();

		if(m_pSurface)
			SDL_FreeSurface(m_pSurface);

		delete m_pRValue;
		delete m_pGValue;
		delete m_pBValue;
	}

	Texture* Texture::LoadFromFile(const std::string& path, ID3D11Device* pDevice)
	{
		auto surface{ IMG_Load(path.c_str()) };
		return new Texture(surface, pDevice);
	}

	ColorRGBA Texture::Sample(const Vector2& uv)
	{
		const Vector2 scaledUV{ std::clamp(uv.x, 0.f, 1.f) * m_pSurface->w, std::clamp(uv.y, 0.f, 1.f) * m_pSurface->h };

		Uint32 pixel{ m_pSurfacePixels[static_cast<int>(scaledUV.y) * m_pSurface->h + static_cast<int>(scaledUV.x)] };

		Uint8 rValue{}, gValue{}, bValue{}, alphaValue{};
		SDL_GetRGBA(pixel, m_pSurface->format, &rValue, &gValue, &bValue, &alphaValue);

		return { rValue / 255.f, gValue / 255.f, bValue / 255.f, alphaValue / 255.f };
	}
}
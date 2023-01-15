#include "pch.h"
#include "Texture.h"
#include "Vector2.h"
#include <cassert>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface, ID3D11Device* pDevice)
		: m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
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

		SDL_FreeSurface(pSurface);
	}

	Texture::~Texture()
	{
		m_pResource->Release();
		m_pSRV->Release();

		delete m_pRValue;
		delete m_pGValue;
		delete m_pBValue;
	}

	Texture* Texture::LoadFromFile(const std::string& path, ID3D11Device* pDevice)
	{
		auto surface{ IMG_Load(path.c_str()) };
		return new Texture(surface, pDevice);
	}

	//ColorRGB Texture::Sample(const Vector2& uv)
	//{
		//const Vector2 scaledUV{ uv.x * m_pSurface->w, uv.y * m_pSurface->h };
		//
		//Uint32 pixel{ m_pSurfacePixels[static_cast<int>(scaledUV.y) * m_pSurface->h + static_cast<int>(scaledUV.x)] };
		//
		//SDL_GetRGB(pixel, m_pSurface->format, m_pRValue, m_pGValue, m_pBValue);
		//
		//return { *m_pRValue / 255.f, *m_pGValue / 255.f, *m_pBValue / 255.f };
	//}
}
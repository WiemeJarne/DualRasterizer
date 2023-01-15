#pragma once

#include <string>
#include "ColorRGB.h"

namespace dae
{
	struct Vector2;

	class Texture
	{
	public:
		~Texture();

		static Texture* LoadFromFile(const std::string& path, ID3D11Device* pDevice);
		//ColorRGB Sample(const Vector2& uv) ;
		ID3D11ShaderResourceView* GetSRV() const { return m_pSRV; }

	private:
		Texture(SDL_Surface* pSurface, ID3D11Device* pDevice);

		uint32_t* m_pSurfacePixels{ nullptr };
		ID3D11ShaderResourceView* m_pSRV{}; // = shader resource view
		ID3D11Texture2D* m_pResource{};
		
		//pixel color to pass to the SDL_GetRGB fucntion
		Uint8 *m_pRValue{ new Uint8() }, *m_pGValue{ new Uint8() }, *m_pBValue{ new Uint8() };
	};
}
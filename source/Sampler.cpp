#include "pch.h"
#include "Sampler.h"
#include <cassert>

Sampler::Sampler(ID3D11Device* pDevice, SamplerStateKind samplerStateKind)
	: m_SamplerStateKind{ samplerStateKind }
{
	D3D11_SAMPLER_DESC samplerDesc{};

	switch (samplerStateKind)
	{
	case SamplerStateKind::point:
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		break;

	case SamplerStateKind::linear:
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		break;

	case SamplerStateKind::anisotropic:
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		break;
	}

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MipLODBias = 0;

	HRESULT result{ pDevice->CreateSamplerState(&samplerDesc, &m_pSamplerState) };

	if (FAILED(result))
		assert("Failed to create samplerState");
}

Sampler::~Sampler()
{
	m_pSamplerState->Release();
}
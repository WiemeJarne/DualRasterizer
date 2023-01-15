#pragma once
#include "pch.h"

class Sampler final
{
public:
	enum class SamplerStateKind
	{
		point,
		linear,
		anisotropic
	};

	Sampler(ID3D11Device* pDevice, SamplerStateKind samplerStateKind);
	~Sampler();
	ID3D11SamplerState* GetSamplerState() const { return m_pSamplerState; }
	SamplerStateKind GetSamplerStateKind() const { return m_SamplerStateKind; }

	Sampler(const Sampler& other) = delete;
	Sampler(Sampler&& other) = delete;
	Sampler& operator=(const Sampler& other) = delete;
	Sampler& operator=(Sampler&& other) = delete;

private:
	ID3D11SamplerState* m_pSamplerState{};
	const SamplerStateKind m_SamplerStateKind{};
};
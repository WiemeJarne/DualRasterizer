#include "pch.h"
#include "PartialCoverageEffect.h"
#include "Texture.h"
#include <cassert>

namespace dae
{
	PartialCoverageEffect::PartialCoverageEffect(ID3D11Device* pDevice, const std::wstring& filePath)
		: Effect(pDevice, filePath)
	{
		m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVariable->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVariable not valid!\n";
		}
	}

	PartialCoverageEffect::~PartialCoverageEffect()
	{
		if (m_pDiffuseMapVariable)
			m_pDiffuseMapVariable->Release();
	}

	void PartialCoverageEffect::SetDiffuseMap(dae::Texture* pDiffuseTexture)
	{
		if (m_pDiffuseMapVariable)
		{
			m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetSRV());
		}
	}
}
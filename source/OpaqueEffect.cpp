#include "pch.h"
#include "OpaqueEffect.h"
#include "Texture.h"
#include <cassert>

namespace dae
{
	OpaqueEffect::OpaqueEffect(ID3D11Device* pDevice, const std::wstring& filePath)
		: Effect(pDevice, filePath)
	{
		m_pSamplerStateVariable = m_pEffect->GetVariableByName("gSamplerState")->AsSampler();
		if (!m_pSamplerStateVariable->IsValid())
		{
			std::wcout << L"m_pSamplerStateVariable not valid!\n";
		}

		m_pWorldMatrixVariable = m_pEffect->GetVariableByName("gWorld")->AsMatrix();
		if (!m_pWorldMatrixVariable->IsValid())
		{
			std::wcout << L"m_pWorldMatrixVariable not valid!\n";
		}

		m_pInvViewMatrix = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
		if (!m_pInvViewMatrix->IsValid())
		{
			std::wcout << L"m_pMatWorldViewProjVariable not valid!\n";
		}

		m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVariable->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVariable not valid!\n";
		}

		m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
		if (!m_pNormalMapVariable->IsValid())
		{
			std::wcout << L"m_pNormalMapVariable not valid!\n";
		}

		m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
		if (!m_pSpecularMapVariable->IsValid())
		{
			std::wcout << L"m_pSpecularMapVariable not valid!\n";
		}

		m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
		if (!m_pGlossinessMapVariable->IsValid())
		{
			std::wcout << L"m_pGlossinessMapVariable not valid!\n";
		}
	}

	OpaqueEffect::~OpaqueEffect()
	{
		if (m_pSamplerStateVariable)
			m_pSamplerStateVariable->Release();

		if (m_pWorldMatrixVariable)
			m_pWorldMatrixVariable->Release();

		if (m_pInvViewMatrix)
			m_pInvViewMatrix->Release();

		if (m_pDiffuseMapVariable)
			m_pDiffuseMapVariable->Release();

		if (m_pNormalMapVariable)
			m_pNormalMapVariable->Release();

		if (m_pSpecularMapVariable)
			m_pSpecularMapVariable->Release();

		if (m_pGlossinessMapVariable)
			m_pGlossinessMapVariable->Release();
	}

	void OpaqueEffect::SetSamplerState(ID3D11SamplerState* pSamplerState)
	{
		if (m_pSamplerStateVariable)
		{
			m_pSamplerStateVariable->SetSampler(0, pSamplerState);
		}
	}

	void OpaqueEffect::SetWorldMatrix(const dae::Matrix& worldMatrix)
	{
		if (m_pWorldMatrixVariable)
		{
			m_pWorldMatrixVariable->SetMatrix(reinterpret_cast<const float*>(&worldMatrix));
		}
	}

	void OpaqueEffect::SetViewInverseMatrix(const dae::Matrix& invViewMatrix)
	{
		if (m_pInvViewMatrix)
		{
			m_pInvViewMatrix->SetMatrix(reinterpret_cast<const float*>(&invViewMatrix));
		}
	}

	void OpaqueEffect::SetDiffuseMap(dae::Texture* pDiffuseTexture)
	{
		if (m_pDiffuseMapVariable)
		{
			m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetSRV());
		}
	}

	void OpaqueEffect::SetNormalMap(dae::Texture* pNormalTexture)
	{
		if (m_pNormalMapVariable)
		{
			m_pNormalMapVariable->SetResource(pNormalTexture->GetSRV());
		}
	}

	void OpaqueEffect::SetSpecularMap(dae::Texture* pSpecularTexture)
	{
		if (m_pSpecularMapVariable)
		{
			m_pSpecularMapVariable->SetResource(pSpecularTexture->GetSRV());
		}
	}

	void OpaqueEffect::SetGlossinessMap(dae::Texture* pGlossinessTexture)
	{
		if (m_pGlossinessMapVariable)
		{
			m_pGlossinessMapVariable->SetResource(pGlossinessTexture->GetSRV());
		}
	}
}
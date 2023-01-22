#include "pch.h"
#include "Effect.h"
#include "Texture.h"

Effect::Effect(ID3D11Device* pDevice, const std::wstring& filePath)
{
	m_pEffect = LoadEffect(pDevice, filePath);

	m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
	if (!m_pTechnique->IsValid())
	{
		std::wcout << L"Technique not valid\n";
	}

	m_pWorldViewProjMatrixVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pWorldViewProjMatrixVariable->IsValid())
	{
		std::wcout << L"m_pMatWorldViewProjVariable not valid!\n";
	}

	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"m_pDiffuseMapVariable not valid!\n";
	}
}

Effect::~Effect()
{
	if (m_pTechnique)
		m_pTechnique->Release();

	if (m_pDiffuseMapVariable)
		m_pDiffuseMapVariable->Release();

	if (m_pInputLayout)
		m_pInputLayout->Release();	

	if (m_pEffect)
		m_pEffect->Release();
}

void Effect::SetWorldViewProjMatrix(const dae::Matrix& worldViewProjMatrix)
{
	if (m_pWorldViewProjMatrixVariable)
		m_pWorldViewProjMatrixVariable->SetMatrix(reinterpret_cast<const float*>(&worldViewProjMatrix));
}

void Effect::SetDiffuseMap(dae::Texture* pDiffuseTexture)
{
	if (m_pDiffuseMapVariable)
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetSRV());
}

ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& filePath)
{
	ID3D10Blob* pErrorBlob{};
	ID3DX11Effect* pEffect;

	DWORD shaderFlags{ 0 };

#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT result
	{
		D3DX11CompileEffectFromFile
		(
			filePath.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob
		)
	};

	if (FAILED(result))
	{
		std::wstringstream stringStream;

		if (pErrorBlob != nullptr)
		{
			const char* pErrors{ static_cast<char*>(pErrorBlob->GetBufferPointer()) };

			for (unsigned int index{}; index < pErrorBlob->GetBufferSize(); ++index)
				stringStream << pErrors[index];

			OutputDebugStringW(stringStream.str().c_str());

			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << stringStream.str() << '\n';
		}
		else
		{
			stringStream << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << filePath << '\n';
			std::wcout << stringStream.str();
			return nullptr;
		}
	}

	return pEffect;
}
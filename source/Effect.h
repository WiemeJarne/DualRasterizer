#pragma once
#include "pch.h"
#include "Matrix.h"

namespace dae
{
	class Texture;
}

class Effect
{
public:
	Effect(ID3D11Device* pDevice, const std::wstring& filePath);
	virtual ~Effect();

	Effect(const Effect& other) = delete;
	Effect(Effect&& other) = delete;
	Effect& operator=(const Effect& other) = delete;
	Effect& operator=(Effect&& other) = delete;

	ID3DX11Effect* GetEffect() const { return m_pEffect; }
	ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; }
	ID3D11InputLayout* GetInputLayout() const { return m_pInputLayout; }
	void SetWorldViewProjMatrix(const dae::Matrix& worldViewProjMatrix);
	void SetDiffuseMap(dae::Texture* pDiffuseTexture);

protected:
	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pTechnique;
	ID3D11InputLayout* m_pInputLayout;
	
	ID3DX11EffectMatrixVariable* m_pWorldViewProjMatrixVariable;
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
	
private:
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& filePath);
};
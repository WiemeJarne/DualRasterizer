#include "pch.h"
#include "Effect.h"

namespace dae
{
	class OpaqueEffect final : public Effect
	{
	public:
		OpaqueEffect(ID3D11Device* pDevice, const std::wstring& filePath);
		~OpaqueEffect();

		OpaqueEffect(const OpaqueEffect& other) = delete;
		OpaqueEffect(OpaqueEffect&& other) = delete;
		OpaqueEffect& operator=(const OpaqueEffect& other) = delete;
		OpaqueEffect& operator=(OpaqueEffect&& other) = delete;

		void SetSamplerState(ID3D11SamplerState* pSamplerState);
		void SetWorldMatrix(const dae::Matrix& worldMatrix);
		void SetViewInverseMatrix(const dae::Matrix& invViewMatrix);
		void SetNormalMap(dae::Texture* pNormalTexture);
		void SetSpecularMap(dae::Texture* pSpecularTexture);
		void SetGlossinessMap(dae::Texture* pGlossinessTexture);

	private:
		ID3DX11EffectSamplerVariable* m_pSamplerStateVariable;
		ID3DX11EffectMatrixVariable* m_pWorldMatrixVariable;
		ID3DX11EffectMatrixVariable* m_pViewInverseMatrixVariable;
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable;
	};
}
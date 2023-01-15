#include "pch.h"
#include "Mesh.h"
#include "Sampler.h"

namespace dae
{
	class OpaqueEffect;

	class OpaqueMesh final : public Mesh
	{
	public:
		OpaqueMesh(ID3D11Device* pDevice, const std::string& modelFilePath, const std::wstring& shaderFilePath);
		~OpaqueMesh();

		OpaqueMesh(const OpaqueMesh& other) = delete;
		OpaqueMesh(OpaqueMesh&& other) = delete;
		OpaqueMesh& operator=(const OpaqueMesh& other) = delete;
		OpaqueMesh& operator=(OpaqueMesh&& other) = delete;

		virtual void Render(ID3D11DeviceContext* pDeviceContext) const override;

		void SetDiffuseMap(Texture* diffuseMap);
		void SetNormalMap(Texture* normalMap);
		void SetSpecularMap(Texture* specularMap);
		void SetGlossinessMap(Texture* glossinessMap);
		void SetWorldViewProjMatrix(const Matrix& worldViewProjMatrix);
		void SetViewInverseMatrix(const Matrix& viewInverseMatrix);
		void ChangeSamplerState(ID3D11Device* pDevice, Sampler* pSampler);
		Sampler::SamplerStateKind GetSamplerStateKind() const { return m_SamplerState; }
		void SetRasterizerState(ID3D11RasterizerState* pRasterizerState);

	private:
		OpaqueEffect* m_pEffect;

		Sampler::SamplerStateKind m_SamplerState;
		ID3D11RasterizerState* m_pRasterizerState{};
	};
}
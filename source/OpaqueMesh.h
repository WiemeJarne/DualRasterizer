#include "pch.h"
#include "Mesh.h"
#include "Sampler.h"

namespace dae
{
	class OpaqueEffect;
	class Texture;

	class OpaqueMesh final : public Mesh
	{
	public:
		OpaqueMesh(ID3D11Device* pDevice, const std::string& modelFilePath, const std::wstring& shaderFilePath, CullMode cullMode, float windowWidth, float windowHeight);
		~OpaqueMesh();

		OpaqueMesh(const OpaqueMesh& other) = delete;
		OpaqueMesh(OpaqueMesh&& other) = delete;
		OpaqueMesh& operator=(const OpaqueMesh& other) = delete;
		OpaqueMesh& operator=(OpaqueMesh&& other) = delete;

		enum class ShadingMode
		{
			combined,
			observedArea,
			diffuse, //(incl. observed area)
			specular //(incl. observed area)
		};

		virtual void RenderInDirectX(ID3D11DeviceContext* pDeviceContext) const override;
		virtual void RenderInSoftwareRasterizer(const Camera& camera, float* pDepthBufferPixels, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) override;
		void SetDiffuseMap(Texture* diffuseMap);
		void SetNormalMap(Texture* normalMap);
		void SetSpecularMap(Texture* specularMap);
		void SetGlossinessMap(Texture* glossinessMap);
		void SetWorldViewProjMatrix(const Matrix& worldViewProjMatrix);
		void SetViewInverseMatrix(const Matrix& viewInverseMatrix);
		void ChangeSamplerState(ID3D11Device* pDevice, Sampler* pSampler);
		Sampler::SamplerStateKind GetSamplerStateKind() const { return m_SamplerState; }
		void SetRasterizerState(ID3D11RasterizerState* pRasterizerState);
		void CycleShadingMode();
		void ToggleUseNormalMap();
		void ToggleDepthBufferVisualization();
		void ToggleBoundingBoxVisualization();

	private:
		OpaqueEffect* m_pEffect;

		Sampler::SamplerStateKind m_SamplerState;
		ID3D11RasterizerState* m_pRasterizerState{};

		Texture* m_pDiffuseMap;
		Texture* m_pNormalMap;
		Texture* m_pSpecularMap;
		Texture* m_pGlossinessMap;

		bool m_UseNormalMap{ true };
		bool m_VisualizeDepthBuffer{};
		bool m_VisualzeBoundingBox{};

		ShadingMode m_ShadingMode{ ShadingMode::combined };

		virtual ColorRGBA ShadePixel(const Vertex_Out& vertex) const override;
	};
}
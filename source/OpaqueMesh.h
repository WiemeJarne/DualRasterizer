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
		enum class CullMode
		{
			BackFace,
			FrontFace,
			None
		};

		OpaqueMesh(ID3D11Device* pDevice, const std::string& modelFilePath, const std::wstring& shaderFilePath, CullMode cullMode, Sampler* pSampler, float windowWidth, float windowHeight);
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

		virtual void RenderHardware(ID3D11DeviceContext* pDeviceContext) const override;
		virtual void RenderSoftware(const Camera& camera, float* pDepthBufferPixels, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) override;
		void SetDiffuseMap(Texture* diffuseMap);
		void SetNormalMap(Texture* normalMap);
		void SetSpecularMap(Texture* specularMap);
		void SetGlossinessMap(Texture* glossinessMap);
		void SetWorldViewProjMatrix(const Matrix& worldViewProjMatrix);
		void SetViewInverseMatrix(const Matrix& viewInverseMatrix);
		void ChangeSamplerState(Sampler* pSampler);
		Sampler::SamplerStateKind GetSamplerStateKind() const { return m_SamplerState; }
		void SetRasterizerState(ID3D11RasterizerState* pRasterizerState);
		void SetCullMode(CullMode cullMode) { m_CullMode = cullMode; }
		void CycleShadingMode();
		void ToggleUseNormalMap();
		void ToggleDepthBufferVisualization();

	private:
		OpaqueEffect* m_pEffect;

		Sampler::SamplerStateKind m_SamplerState;
		ID3D11RasterizerState* m_pRasterizerState{ nullptr };
		
		Texture* m_pDiffuseMap{ nullptr };
		Texture* m_pNormalMap{ nullptr };
		Texture* m_pSpecularMap{ nullptr };
		Texture* m_pGlossinessMap{ nullptr };

		bool m_UseNormalMap{ true };
		bool m_VisualizeDepthBuffer{};

		ShadingMode m_ShadingMode{ ShadingMode::combined };

		CullMode m_CullMode{};

		int amount{};

		bool ShouldRenderTriangle(CullMode cullMode, float area) const;
		virtual void RenderTriangle(uint32_t triangleIndex, const Vertex_Out& vertex0, const Vertex_Out& vertex1, const Vertex_Out& vertex2, float triangleArea, float* pDepthBufferPixels, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) const override;
		virtual void RenderPixel(uint32_t pixelIndex, const Vertex_Out& triangleVertex0, const Vertex_Out& triangleVertex1, const Vertex_Out& triangleVertex2, float triangleArea, float* pDepthBufferPixels, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) const override;
		virtual ColorRGBA ShadePixel(const Vertex_Out& vertex) const override;
	};
}
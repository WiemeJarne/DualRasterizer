#include "pch.h"
#include "Mesh.h"

namespace dae
{
	class PartialCoverageEffect;
	class Texture;

	class PartialCoverageMesh final : public Mesh
	{
	public:
		PartialCoverageMesh(ID3D11Device* pDevice, const std::string& modelFilePath, const std::wstring& shaderFilePath, float windowWidth, float windowHeight);
		~PartialCoverageMesh();

		PartialCoverageMesh(const PartialCoverageMesh& other) = delete;
		PartialCoverageMesh(PartialCoverageMesh&& other) = delete;
		PartialCoverageMesh& operator=(const PartialCoverageMesh& other) = delete;
		PartialCoverageMesh& operator=(PartialCoverageMesh&& other) = delete;

		virtual void RenderHardware(ID3D11DeviceContext* pDeviceContext) const override;
		virtual void RenderSoftware(const Camera& camera, float* pDepthBufferPixels, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) override;
		void SetDiffuseMap(Texture* diffuseMap);
		void SetWorldViewProjMatrix(const Matrix& worldViewProjMatrix);

	private:
		PartialCoverageEffect* m_pEffect;
		Texture* m_pDiffuseMap{ nullptr };

		virtual ColorRGBA ShadePixel(const Vertex_Out& vertex) const override;
	};
}
#include "pch.h"
#include "Mesh.h"

namespace dae
{
	class PartialCoverageEffect;

	class PartialCoverageMesh final : public Mesh
	{
	public:
		PartialCoverageMesh(ID3D11Device* pDevice, const std::string& modelFilePath, const std::wstring& shaderFilePath);
		~PartialCoverageMesh();

		PartialCoverageMesh(const PartialCoverageMesh& other) = delete;
		PartialCoverageMesh(PartialCoverageMesh&& other) = delete;
		PartialCoverageMesh& operator=(const PartialCoverageMesh& other) = delete;
		PartialCoverageMesh& operator=(PartialCoverageMesh&& other) = delete;

		virtual void Render(ID3D11DeviceContext* pDeviceContext) const override;

		void SetDiffuseMap(Texture* diffuseMap);
		void SetWorldViewProjMatrix(const Matrix& worldViewProjMatrix);

	private:
		PartialCoverageEffect* m_pEffect;
	};
}
#pragma once
#include "pch.h"
#include "Effect.h"

namespace dae
{
	class Texture;

	class Mesh
	{
	public:
		struct Vertex_PosTex
		{
			Vector3 position;
			Vector3 normal;
			Vector3 tangent;
			Vector2 uv;
		};

		Mesh(ID3D11Device* pDevice, const std::string& modelFilePath);
		virtual ~Mesh();

		Mesh(const Mesh& other) = delete;
		Mesh(Mesh&& other) = delete;
		Mesh& operator=(const Mesh& other) = delete;
		Mesh& operator=(Mesh&& other) = delete;

		virtual void Render(ID3D11DeviceContext* pDeviceContext) const = 0;
		void RotateYCW(float angle); //CW = clockwise
		const Matrix& GetWorldMatrix() const { return m_WorldMatrix; }
		float GetRotationSpeed() const { return m_RotationSpeed; }
		float GetRotationAngle() const { return m_RotationAngle; }

	protected:
		Matrix m_WorldMatrix
		{
			{1, 0, 0, 0},
			{0, 1, 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 1}
		};

		ID3D11Buffer* m_pVertexBuffer{};
		ID3D11Buffer* m_pIndexBuffer{};

		uint32_t m_AmountOfIndices{};

	private:
		float m_RotationAngle{};
		float m_RotationSpeed{};
	};
}
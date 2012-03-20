#pragma once
#include "CrazeMath.h"

#include "D3D11.h"

namespace Craze
{
	namespace Graphics2
	{
		struct CBPerLight
		{
			Matrix4 lightViewProj;
			Matrix4 lightWorld;
			Vec3 lightColor;
			float lightSpecular;
			Vec3 lightPos;
			float lightRange;
			Vec3 lightDir;
			float lightAngle;
			unsigned int lightType;
			float texelSize;
			Vector2 invLightMapRes;
		};

		struct CBPerFrame
		{
			Vector4 cameraPos;
			Matrix4 viewProj;
			Vector2 invResolution;
			Vec3 ambientColor;
			Vec3 LVStart;
			Vec3 LVEnd;
			Vector4 LVCellSize;
			bool DisableDirect;
			bool DisableAO;
			bool DisableIndirect;
			bool BoostIndirect;
			bool UseESM;
			unsigned int NumObjects;
		};

		struct CBPerObject
		{
			Matrix4 world;
			Matrix3 normalTfm;
			float specularFactor;
			unsigned int lightMapIndex;
		};


		class Device;
		class SpotLight;
		class DirectionalLight;

		class CBufferManager
		{
		private:
			ID3D11Buffer* m_pPerLight;
			ID3D11Buffer* m_pPerFrame;
			ID3D11Buffer* m_pPerObject;

			Device* m_pDevice;

		public:
			CBufferManager();
			~CBufferManager();

			void Initialize(Device* pDevice);
			void Shutdown();

			void ResetBuffers();

			static CBPerLight GetLightData(const DirectionalLight& light);

			void SetFrame(const CBPerFrame& data);
			void SetLight(const CBPerLight& data);
			void SetLight(const SpotLight& light, float texelSize = 0.0f);
			void SetLight(const DirectionalLight& light, float texelSize = 0.0f);
			void SetLight(const DirectionalLight& light, const Matrix4& viewProj, Vec3 lightPos, float texelSize = 0.0f);

			void SetObject(const CBPerObject& data);
		};
	}
}

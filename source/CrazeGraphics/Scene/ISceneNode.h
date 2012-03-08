#pragma once

namespace Craze
{
	namespace Graphics2
	{
		class DisplayTree;


		class ISceneNode
		{
		public:
			ISceneNode() { m_pBoundingBox = Mesh::s_BBoxPool.malloc(); }
			virtual void VDraw(DrawList* pLst, float depth) = 0;

			const BoundingBox* GetBoundingBox() const { return m_pBoundingBox; }
			virtual void VSetTransform(const Matrix4& transform) = 0;

			virtual ~ISceneNode() 
			{
				Mesh::s_BBoxPool.free(m_pBoundingBox);
			}
			
		protected:
			bool remove : 1;
		};
	}
}
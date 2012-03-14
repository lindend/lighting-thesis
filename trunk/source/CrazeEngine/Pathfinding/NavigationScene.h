#pragma once
#include <memory>

#include "Memory/MemoryManager.h"
#include "NavInputMesh.h"

#include "Path.h"

struct rcPolyMesh;
struct rcPolyMeshDetail;
class dtNavMesh;
class dtNavMeshQuery;

namespace Craze
{
	namespace Graphics2
	{
		class Mesh;
	}

	//The params for creating the navigation scene:
	extern float AgentHeight;
	extern float AgentClimb ;
	extern float AgentWidth;
	extern int MaxEdgeLen;
	extern float MaxEdgeError;
	extern int MinRegionSize;
	extern int MergedRegionSize;
	extern int MaxVertsPerPoly;
	extern float DetailSampleDist;
	extern float CellSize;
	extern float CellHeight;

	class NavigationScene
	{
		CRAZE_ALLOC();
	public:
		NavigationScene();
		~NavigationScene();

		//Adds a mesh to the navigation scene, the memory will be owned by the NavigationScene instance. The memory will be freed when ClearScene is called, or the NavigationScene is destroyed.
		//The effect of the mesh will be part of the walk mesh once BuildNavigationMesh has been called.
		void AddMesh(const NavInputMesh* pMesh);
		//Removes a mesh from the navigation scene, and the memory is no longer owned by the NavigationScene instance.
		//The effect of the mesh will be removed from the walk mesh once BuildNavigationMesh has been called
		void RemoveMesh(const NavInputMesh* pMesh);
		//If the specified mesh exists in the scene, it is returned; otherwise NULL is returend.
		const NavInputMesh* GetMesh(const NavInputMesh* pMesh) const;

		std::shared_ptr<Graphics2::Mesh> CreateGraphicsMesh() const;

		bool Build();

		bool FindPath(const Vec3& start, const Vec3& end, Path* pOutPath);

		void ClearScene();
	private:
		bool CreateRecastMeshes();
		bool CreateDetourMesh();

		void CleanUp();
		std::vector<const NavInputMesh*> m_pMeshes;

		rcPolyMesh* m_pMesh;
		rcPolyMeshDetail* m_pMeshDetail;
		dtNavMesh* m_pNavMesh;
		dtNavMeshQuery* m_pNavQuery;

		NavigationScene(const NavigationScene&);
		NavigationScene& operator=(const NavigationScene&);
	};
}
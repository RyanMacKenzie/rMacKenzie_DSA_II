#ifndef _MYOCTANTCLASS_H_
#define _MYOCTANTCLASS_H_

#include "MyEntityManager.h"

namespace Simplex
{
class MyOctant
{
	static uint m_uOctantCount; //number of octants
	static uint m_uMaxLevel; //octant max level

	uint m_uID = 0; //octant ID
	uint m_uLevel = 0; //octant level in octree
	uint m_uChildren = 0; //whether this octant has 0 or 8 children

	float m_fSize = 0.0f; //octant's size in world units

	MeshManager* m_pMeshMngr = nullptr; //mesh manager pointer
	MyEntityManager* m_pEntityMngr = nullptr; //entity manager pointer

	vector3 m_v3Center = vector3(0.0f); //center point
	vector3 m_v3Min = vector3(0.0f); //max point
	vector3 m_v3Max = vector3(0.0f); //min point

	MyOctant* m_pParent = nullptr; //this octant's parent. we makin' a tree, lads
	MyOctant* m_pChildren[8]; //will contain this node's children

	std::vector<uint> m_EntityList; //c++ is a pile of garbage. and why does this just get a x_ prefix? OPENGL EXPLAIN

	MyOctant* m_pRoot = nullptr; //root octant
	std::vector<MyOctant*> m_lChild; //list of nodes that contain objects, for use by root node

public:
	MyOctant(uint a_nMaxLevel = 2); //root constructor

	MyOctant(vector3 a_vCenter, float a_fSize); //node constructor

	MyOctant(MyOctant const& other); //copy constructor

	MyOctant& operator=(MyOctant const& other); //copy assignment operator

	~MyOctant(); //destructor

	void Swap(MyOctant& other); //swap this object's contents with another's

	float GetSize(void); //get size of octant in global space

	vector3 GetCenterGlobal(void); //return the center vector in global space

	vector3 GetMinGlobal(void); //return the minimum vector in global space

	vector3 GetMaxGlobal(void); //return the maximum vector in global space

	bool IsColliding(uint a_uRBIndex); //returns true if the octant is colliding with the object at the given index - read: returns true if the object is in this octant

	void Display(uint a_nIndex, vector3 a_v3Color = C_YELLOW); //display the octant at the given index in the given color

	void Display(vector3 a_v3Color = C_YELLOW); //displays this octant in the color specified

	void DisplayLeafs(vector3 a_v3Color = C_YELLOW); //displays all leafs of this octant that contain entities

	void ClearEntityList(void); //clears the entity list for ALL nodes

	void Subdivide(void); //creates 8 children (0-7) in this octant's space

	MyOctant* GetChild(uint a_nChild); //get the specified child of this octant

	MyOctant* GetParent(void); //get this octant's parent

	bool IsLeaf(void); //true if this octant has no children

	void KillBranches(void); //delete everything underneath this octant

	void ConstructTree(uint a_nMaxLevels = 3); //creates an octree with the indicated number of levels

	void AssignIDtoEntity(void); //sets IDs of leaves

	uint GetOctantCount(void); //gets total world octants

private:
	void Release(void); //deallocates class members

	void Init(void); //allocates class members

	void ConstructList(void); //creates list of all leaves containing objects

};
}

#endif
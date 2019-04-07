#include "MyOctant.h"

using namespace Simplex;

//why can you even do this
uint MyOctant::m_uOctantCount = 0;
uint MyOctant::m_uMaxLevel = 3;

uint MyOctant::GetOctantCount(void)
{
	return m_uOctantCount;
}

void MyOctant::Init(void)
{
	m_uID = m_uOctantCount; //id
	
	m_uChildren = 0; //zeros
	m_fSize = 0;
	m_uLevel = 0;

	m_v3Center = vector3(0.0f); //zero vectors
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	m_pMeshMngr = MeshManager::GetInstance(); //mngr references
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_pRoot = nullptr; //nullptrs. because this is C++ and you can't just have "null". Seriously, can I go back to C# yet?
	m_pParent = nullptr; //the x_y syntax gives me an aneurysmmmmmmmmmmmmmmmm
	for (int i = 0; i < 8; i++)
	{
		m_pChildren[i] = nullptr;
	}
}

void MyOctant::Swap(MyOctant& other)
{
	std::swap(m_uChildren, other.m_uChildren); //some of this stuff is order-dependent
	std::swap(m_fSize, other.m_fSize);
	std::swap(m_uID, other.m_uID);
	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_lChild, other.m_lChild);
	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);

	m_pMeshMngr = MeshManager::GetInstance(); //mngr references
	m_pEntityMngr = MyEntityManager::GetInstance();

	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_pParent, other.m_pParent);

	for (int i = 0; i < 8; i++)
	{
		std::swap(m_pChildren[i], other.m_pChildren[i]); //this std::swap method feels like cheating
	}
}

MyOctant * MyOctant::GetParent(void)
{
	return m_pParent;
}

void MyOctant::Release(void)
{
	if (m_uLevel == 0)
	{
		KillBranches(); //Execute order 66
	}

	m_uChildren = 0;
	m_fSize = 0;
	m_EntityList.clear();
	m_lChild.clear();
}

//NUMBAH ONE
MyOctant::MyOctant(uint a_nMaxLevel)
{
	Init(); //Initialize to default. We do this in 2-3 different places because we're using C++ like cavemen. C and C# are the alpha languages.

	m_uOctantCount = 0;
	m_uMaxLevel = a_nMaxLevel;
	m_uID = m_uOctantCount;

	m_pRoot = this;
	m_lChild.clear();

	std::vector<vector3> lMinMax; //min/max values of BO

	uint numObjects = m_pEntityMngr->GetEntityCount(); //I'm naming this one variable something that makes sense and you can't stop me
	for (int i = 0; i < numObjects; i++)
	{
		MyEntity* pEntity = m_pEntityMngr->GetEntity(i);
		MyRigidBody* pRigidBody = pEntity->GetRigidBody();
		lMinMax.push_back(pRigidBody->GetMinGlobal()); //push min/max in consistent order so they can be grabbed later from the same list
		lMinMax.push_back(pRigidBody->GetMaxGlobal());
	}

	MyRigidBody* pRigidBody = new MyRigidBody(lMinMax); //voodoo magic make a big box i guess

	vector3 vHalfWidth = pRigidBody->GetHalfWidth();
	float fMax = vHalfWidth.x;
	for (int i = 1; i < 3; i++) //make sure the fMax really is max
	{
		if (fMax < vHalfWidth[i])
		{
			fMax = vHalfWidth[i];
		}
	}

	vector3 vCenter = pRigidBody->GetCenterLocal();
	lMinMax.clear(); //clean up array
	SafeDelete(pRigidBody); //"SafeDelete" always sounds like something that can't possibly be safe to me

	m_fSize = fMax * 2.0f;
	m_v3Center = vCenter;
	m_v3Min = m_v3Center - (vector3(fMax));
	m_v3Max = m_v3Center + (vector3(fMax));

	m_uOctantCount++;

	ConstructTree(m_uMaxLevel);
}

//NUMBAH 2
MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	Init();

	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	m_v3Min = m_v3Center - (vector3(m_fSize / 2.0f));
	m_v3Max = m_v3Center + (vector3(m_fSize / 2.0f));

	m_uOctantCount++;
}

//COPY BOI
MyOctant::MyOctant(MyOctant const& other)
{
	m_uChildren = other.m_uChildren;
	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;

	m_fSize = other.m_fSize;
	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;
	m_pParent = other.m_pParent;

	m_pRoot = other.m_pRoot;
	m_lChild = other.m_lChild;

	m_pMeshMngr = MeshManager::GetInstance(); //mngr references
	m_pEntityMngr = MyEntityManager::GetInstance();

	for (int i = 0; i < 8; i++)
	{
		m_pChildren[i] = other.m_pChildren[i];
	}
}

//ASSIGNMENT BOI
MyOctant& MyOctant::operator=(MyOctant const& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}
	return *this;
}

//Accessors - not really a yoink/yeet because the values are retained
float MyOctant::GetSize(void)
{
	return m_fSize;
}

vector3 MyOctant::GetCenterGlobal(void)
{
	return m_v3Center;
}

vector3 MyOctant::GetMinGlobal(void)
{
	return m_v3Min;
}

vector3 MyOctant::GetMaxGlobal(void)
{
	return m_v3Max;
}

//Bread & Butter - the functionality to take advantage of that sweet core structure
void MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	if (m_uID == a_nIndex) //fork here if this is the correct octant to display
	{
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
		return;
	}
	else
	{
		for (int i = 0; i < m_uChildren; i++)
		{
			m_pChildren[i]->Display(a_nIndex); //recursion... how long it has been. I'd almost be willing to take a semester on this stuff. almost.
		}
	}
}

void MyOctant::Display(vector3 a_v3Color)
{
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChildren[i]->Display(a_v3Color); //deja vu
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE); //i just been in this place before
}

void MyOctant::Subdivide(void)
{
	if (m_uLevel >= m_uMaxLevel) //if max depth, don't
	{
		return;
	}

	if (m_uChildren != 0) //if already divided, don't
	{
		return;
	}

	m_uChildren = 8; //congrats, dad

	float fSize = m_fSize / 4.0f;
	float fSizeD = fSize * 2.0f;

	vector3 v3Center;

	v3Center = m_v3Center;
	v3Center.x -= fSize;
	v3Center.y -= fSize;
	v3Center.z -= fSize;
	m_pChildren[0] = new MyOctant(v3Center, fSizeD);

	v3Center.x += fSizeD;
	m_pChildren[0] = new MyOctant(v3Center, fSizeD);

	v3Center.z += fSizeD;
	m_pChildren[0] = new MyOctant(v3Center, fSizeD);

	v3Center.x -= fSizeD;
	m_pChildren[0] = new MyOctant(v3Center, fSizeD);

	v3Center.y += fSizeD;
	m_pChildren[0] = new MyOctant(v3Center, fSizeD);

	v3Center.z -= fSizeD;
	m_pChildren[0] = new MyOctant(v3Center, fSizeD);

	v3Center.x += fSizeD;
	m_pChildren[0] = new MyOctant(v3Center, fSizeD);

	v3Center.z += fSizeD;
	m_pChildren[0] = new MyOctant(v3Center, fSizeD);

	for (int i = 0; i < 8; i++)
	{
		m_pChildren[i]->m_pRoot = m_pRoot;
		m_pChildren[i]->m_pParent = this;
		m_pChildren[i]->m_uLevel = m_uLevel + 1;
	}
}

MyOctant * MyOctant::GetChild(uint a_nChild)
{
	if (a_nChild > 7) //can't have more than 8 children
	{
		return nullptr;
	}

	return m_pChildren[a_nChild];
}

bool MyOctant::IsColliding(uint a_uRBIndex)
{
	uint nObjectCount = m_pEntityMngr->GetEntityCount();

	if (a_uRBIndex > nObjectCount)
	{
		return false; //can't collide with something that doesn't exist
	}

	MyEntity* pEntity = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* pRigidBody = pEntity->GetRigidBody();
	vector3 v3MinO = pRigidBody->GetMinGlobal();
	vector3 v3MaxO = pRigidBody->GetMaxGlobal();

	if (m_v3Max.x < v3MinO.x)
	{
		return false;
	}
	if (m_v3Min.x > v3MaxO.x)
	{
		return false;
	}

	if (m_v3Max.y < v3MinO.y)
	{
		return false;
	}
	if (m_v3Min.y > v3MaxO.y)
	{
		return false;
	}

	if (m_v3Max.z < v3MinO.z)
	{
		return false;
	}
	if (m_v3Min.z > v3MaxO.z)
	{
		return false;
	}

	return true; //if we're not not colliding, we must be colliding
}

bool MyOctant::IsLeaf(void)
{
	return m_uChildren == 0;
}

//Order 66
void MyOctant::KillBranches(void)
{
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChildren[i]->KillBranches();
		delete m_pChildren[i];
		m_pChildren[i] = nullptr;
	}
	m_uChildren = 0;
}

void MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	uint nLeafs = m_lChild.size();
	for (int i = 0; i < nLeafs; i++)
	{
		m_lChild[i]->DisplayLeafs(a_v3Color);
	}

	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE); //HIGHER ON THE STREETS
}


void MyOctant::ClearEntityList(void)
{
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChildren[i]->ClearEntityList();
	}
	m_EntityList.clear();
}

void MyOctant::ConstructTree(uint a_nMaxLevel)
{
	if (m_uLevel != 0) //root node only
	{
		return;
	}

	m_uMaxLevel = a_nMaxLevel;

	m_uOctantCount = 1;

	m_EntityList.clear();

	KillBranches();
	m_lChild.clear();

	AssignIDtoEntity();

	ConstructList();
}

void MyOctant::AssignIDtoEntity(void)
{
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChildren[i]->AssignIDtoEntity();
	}
	if (m_uChildren == 0)
	{
		uint nEntities = m_pEntityMngr->GetEntityCount();
		for (int i = 0; i < nEntities; i++)
		{
			if (IsColliding(i))
			{
				m_EntityList.push_back(i);
				m_pEntityMngr->AddDimension(i, m_uID);
			}
		}
	}
}

void MyOctant::ConstructList(void)
{
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChildren[i]->ConstructList();
	}

	if (m_EntityList.size() > 0)
	{
		m_pRoot->m_lChild.push_back(this);
	}
}
#include "PBRMeshObject.h"
#include <framework.h>
#include <Utility/SerializedUtility.h>

void PBRMeshObject::Awake()
{
	
}

void PBRMeshObject::Serialized(std::ofstream& ofs)
{
	using namespace Binary;
	size_t MeshCount = 0;
	for (size_t i = 0; i < GetComponentCount(); i++)
	{
		if (GetComponentAtIndex<PBRMeshRender>(i))
		{
			MeshCount++;
		}
	}
	if (MeshCount > 0)
	{
		Write::data(ofs, MeshCount);
		Write::string(ofs, typeid(PBRMeshRender).raw_name());
	}
	else
	{
		size_t boneMeshCount = 0;
		for (size_t i = 0; i < GetComponentCount(); i++)
		{
			if (GetComponentAtIndex<PBRBoneMeshRender>(i))
			{
				boneMeshCount++;
			}
		}
		if (boneMeshCount > 0)
		{
			Write::data(ofs, boneMeshCount);
			Write::string(ofs, typeid(PBRBoneMeshRender).raw_name());
		}
		else
		{
			Write::data(ofs, size_t{0});
		}
	}
}

void PBRMeshObject::Deserialized(std::ifstream& ifs)
{
	using namespace Binary;
	size_t meshCount = Read::data<size_t>(ifs);
	if (meshCount > 0)
	{
		std::string type = Read::string(ifs);
		if (type == typeid(PBRMeshRender).raw_name())
		{
			for (size_t i = 0; i < meshCount; i++)
			{
				AddComponent<PBRMeshRender>();
			}
		}
		else
		{
			for (size_t i = 0; i < meshCount; i++)
			{
				AddComponent<PBRBoneMeshRender>();
			}
		}
	}
}


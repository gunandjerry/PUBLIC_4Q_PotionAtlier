#include "EmptyTable.h"
#include "PlayerController.h"
#include "Holding.h"

EmptyTable::EmptyTable()
{
	type = InteractableType::EmptyTable;
}

void EmptyTable::Start()
{
	if (above_mesh == nullptr)
	{
		auto* holdingObj = FindHoldingObject(&transform);
		if (holdingObj != nullptr)
		{
			above_mesh = holdingObj->IsComponent<Holding>();
		}
	}
}

void EmptyTable::InspectorImguiDraw()
{
}

void EmptyTable::Serialized(std::ofstream& ofs)
{
	constexpr size_t header = (std::numeric_limits<size_t>::max)();
	constexpr uint32_t version = 0;
	Binary::Write::data(ofs, header); //헤더
	Binary::Write::data(ofs, version); //버전

	Binary::Write::data<bool>(ofs, something_on);
	Binary::Write::data<UINT>(ofs, static_cast<UINT>(hold_type));
	Binary::Write::data<UINT>(ofs, sub_type);
}

void EmptyTable::Deserialized(std::ifstream& ifs)
{
	size_t header = Binary::Read::data<size_t>(ifs);
	uint32_t version = 0;
	if (header != (std::numeric_limits<size_t>::max)())
	{
	}
	else
	{
		version = Binary::Read::data<uint32_t>(ifs);
	}

	something_on = Binary::Read::data<bool>(ifs);
	hold_type = static_cast<HoldableType>(Binary::Read::data<UINT>(ifs));
	sub_type = Binary::Read::data<UINT>(ifs);
}

void EmptyTable::OnFocusIn(PlayerController* controller)
{

}
void EmptyTable::OnFocusOut(PlayerController* controller)
{

}
bool EmptyTable::OnInteract(PlayerController* controller)
{
	if (above_mesh == nullptr)
	{
		auto* holdingObj = FindHoldingObject(&transform);
		if (holdingObj != nullptr)
		{
			above_mesh = holdingObj->IsComponent<Holding>();
		}
	}

	if (something_on == false && controller->something_on_hand == false)
	{
		return false;
	}
	else if (something_on == true && controller->something_on_hand == true)
	{
		return false;

		/*std::pair<HoldableType, UINT> own = controller->Swap(hold_type, sub_type);

		bool result = above_mesh->SetType(own.first, own.second);
		if (result == false)
		{
			printf("Trying to pick undefined ingredient.");
			__debugbreak;
			return false;
		}
		else
		{
			hold_type = own.first;
			sub_type = own.second;
		}

		return true;*/
	}
	else if (something_on == true && controller->something_on_hand == false)
	{
		controller->Pick(hold_type, sub_type);
		above_mesh->SetEmpty();
		hold_type = HoldableType::None;
		sub_type = 0;

		something_on = false;
		return true;
	}
	else if (something_on == false && controller->something_on_hand == true)
	{
		hold_type = controller->hold_type;
		sub_type = controller->hold_subtype;
		above_mesh->SetType(hold_type, sub_type);

		something_on = true;
		controller->PutDown();

		return true;
	}
}

GameObject* EmptyTable::FindHoldingObject(Transform* parent)
{
	auto num = parent->GetChildCount();
	for (int i = 0; i < num; ++i)
	{
		Transform* c = parent->GetChild(i);
		if (c->gameObject.HasTag(L"HoldingObject"))
		{
			return &c->gameObject;
		}
		GameObject* obj = FindHoldingObject(c);
		if (obj != nullptr) return obj;
	}

	return nullptr;
}

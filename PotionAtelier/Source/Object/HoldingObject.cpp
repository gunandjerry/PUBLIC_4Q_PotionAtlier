#include "HoldingObject.h"
#include "../Components/Holding.h"

void HoldingObject::Awake()
{
	AddComponent<Holding>();

	//auto& tr = AddComponent<TextRender>();
	


	//CubeObject::Awake();
	//auto& aa = GetComponent<CubeMeshRender>();
	//aa.materialAsset.GetAssetData()->customData.SetField("WTF", Color{ 1, 0, 0, 1 });

	/*auto& com = AddComponent<TextRender>();
	com.SetText(L"동해물과백두산이마르고닳도록\n시진핑핑이"); <- ??? 누구냐 이거
	com.SetSize(30);
	com.SetColor({ 1, 0, 0, 1 });
	com.SetPosition({ 900, 600 });
	com.SetType(FontType::Ulsan);*/

	/*auto& ui = AddComponent<UIRenderComponenet>();
	ui.SetTransform(300, 300, 300, 300);
	ui.SetTexture(L"./Resource/Sample/Item_Flower.png");

	auto& ev = AddComponent<EventListener>();
	ev.SetOnClickFunction([]()
	{
		printf("I'm clicked!\n");
	});*/
}

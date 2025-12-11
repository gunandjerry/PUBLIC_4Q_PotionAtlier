#include "CustomerState.h"
#include <Components/CustomerAI.h>
#include <Object\GameManager.h>
#include <ResourceFinder.h>
#include <StringResource.h>
#include <Components/TextBubble.h>

void RunState::Enter(CustomerAI* customer)
{
	customer->SetAnimationClip(GnomeAnimType::Run);
}

static bool CheckFever(CustomerAI* customer)
{
	if (GameManager::GetGM().IsFever())
	{
		customer->ChangeState(CustomerStateType::Fever);
		return true;
	}
	return false;
}

void RunState::Update(CustomerAI* customer)
{
	if (CheckFever(customer))
		return;

	if (customer->isOrderEnd)
	{
		customer->Move();
	}
	else if (customer->IsStopPoint())
	{
		customer->ChangeState(CustomerStateType::Order);
	}
	else if (GameManager::GetGM().GetOrderCount() < 5)
	{
		customer->Move();
	}
	else if (customer->RayForward() == true)
	{
		customer->ChangeState(CustomerStateType::Wait);
	}
	else
	{
		customer->Move();
	}
}



void WaitState::Enter(CustomerAI* customer)
{
	customer->SetAnimationClip(GnomeAnimType::Idle);
}

void WaitState::Update(CustomerAI* customer)
{
	if (customer->RayForward() == false)
	{
		customer->ChangeState(CustomerStateType::Run);
	}

	CheckFever(customer);
}


void OrderState::Enter(CustomerAI* customer)
{
	id = -1;
	t = customer->max_patience;
	text_step = 0;
	customer->SetAnimationClip(GnomeAnimType::Idle);
	player = GameObject::Find(L"Player");
}

void OrderState::Update(CustomerAI* customer)
{
	if (CheckFever(customer))
		goto label_RETURN;

	if (id == -1)
	{
		//피버 종료시 순차적 주문
		if (customer->GetPrevStateType() == CustomerStateType::Fever && customer->myOrderQueueIndex != GameManager::GetGM().GetOrderCount())
			goto label_RETURN;

		//주문 시도
		id = GameManager::GetGM().OrderPotion(
			customer->GetPotion(),
			std::bind(&CustomerAI::OnSuccess, customer),
			std::bind(&CustomerAI::OnFailed, customer));

		if (id > -1)
		{
			customer->myOrderQueueIndex = GameManager::GetGM().GetOrderCount() - 1;
		}			
	}
	else
	{
		t -= TimeSystem::Time.DeltaTime;
		GameManager::GetGM().OrderSheetUpdate(id, t / customer->max_patience);
		if (prevOrderQueueCount > GameManager::GetGM().GetOrderCount())
		{
			if (customer->myOrderQueueIndex > 0)
			{
				customer->myOrderQueueIndex--;
				customer->isSortEnd = false;
				customer->SetAnimationClip(GnomeAnimType::Run);
			}	
		}




		if (text_step == 0 && t <= customer->max_patience * 0.6777f)
		{
			TextBubble& bubble = customer->GetComponent<TextBubble>();
			static float bubble_drawSpeed{ 0.0f };
			bubble.SetDrawSpeed(bubble_drawSpeed);
			bubble_drawSpeed += 0.3f;
			if (bubble_drawSpeed >= 1.5f)
			{
				bubble_drawSpeed = 0.0f;
			}

			bubble.ShowBubble();
			int i = Random::Range(3, 5);
			bubble.SetBubbleText(StringResource::GetTutorialText(std::format(L"CT_{}", i)));
			++text_step;

			TimeSystem::Time.DelayedInvok([&bubble]()
			{
				bubble.HideBubble();
			}, TextBubble::text_bubble_duration_common);
		}
		else if (text_step == 1 && t <= customer->max_patience * 0.377f)
		{
			TextBubble& bubble = customer->GetComponent<TextBubble>();
			bubble.ShowBubble();
			int i = Random::Range(6, 8);
			bubble.SetBubbleText(StringResource::GetTutorialText(std::format(L"CT_{}", i)));
			++text_step;

			TimeSystem::Time.DelayedInvok([&bubble]()
			{
				bubble.HideBubble();
			}, TextBubble::text_bubble_duration_common);
		}





		if(customer->isSortEnd == false)
			customer->MoveSort();
		else
		{
			if (player)
			{
				Vector3 direction = customer->transform.position - player->transform.position;
				direction.y = 0;
				direction.Normalize();
				if (direction.Dot(Vector3::Up) > 1.f - Mathf::Epsilon)
				{
					customer->transform.rotation = Quaternion::LookRotation(
						direction,
						Vector3::Right);
				}
				else
				{
					customer->transform.rotation = Quaternion::LookRotation(
						direction,
						Vector3::Up);
				}
			}
		}
	}
label_RETURN:
	prevOrderQueueCount = GameManager::GetGM().GetOrderCount();
	return;
}


void HitState::Enter(CustomerAI* customer)
{
	customer->SetAnimationClip(GnomeAnimType::Hit);
}

void HitState::Update(CustomerAI* customer)
{

}


void FeverState::Enter(CustomerAI* customer)
{
	//원래 춤춰야함
	customer->SetAnimationClip(GnomeAnimType::Dance);
}

void FeverState::Update(CustomerAI* customer)
{
	if (!GameManager::GetGM().IsFever())
	{
		TimeSystem::Time.DelayedInvok(
			[customer]()
			{
  				customer->ChangePrevState();
			},
			0.f);
	}		
}

void ExitState::Enter(CustomerAI* customer)
{
	
}

void ExitState::Update(CustomerAI* customer)
{
	
}

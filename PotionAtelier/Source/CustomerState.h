#pragma once


class CustomerAI;

enum class CustomerStateType
{
	Run,
	Wait, 
	Order,
	Hit, 
	Fever, 
	Exit,
};

class CustomerState abstract
{
public:
	virtual ~CustomerState() = default;

public:
	virtual void Enter(CustomerAI* customer) = 0;
	virtual void Update(CustomerAI* customer) = 0;
};

//===========================================================
class RunState : public CustomerState
{
public:
	void Enter(CustomerAI* customer) override;
	void Update(CustomerAI* customer) override;

};

class WaitState : public CustomerState
{
public:
	void Enter(CustomerAI* customer) override;
	void Update(CustomerAI* customer) override;

};

class OrderState : public CustomerState
{
public:
	void Enter(CustomerAI* customer) override;
	void Update(CustomerAI* customer) override;

	float t = 0;
	int id = 0;
	int prevOrderQueueCount = 0;
	int text_step = 0;
	class GameObject* player = nullptr;
};

class HitState : public CustomerState
{
public:
	void Enter(CustomerAI* customer) override;
	void Update(CustomerAI* customer) override;

};

class FeverState : public CustomerState
{
public:
	void Enter(CustomerAI* customer) override;
	void Update(CustomerAI* customer) override;

};

class ExitState : public CustomerState
{
public:
	void Enter(CustomerAI* customer) override;
	void Update(CustomerAI* customer) override;

	float exitcurrTime;

};
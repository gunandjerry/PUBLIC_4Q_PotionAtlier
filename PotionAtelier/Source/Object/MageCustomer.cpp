#include "MageCustomer.h"
#include <Components/CustomerAI.h>
#include <Component/CharacterController/CharacterController.h>
#include "../Components/BoingBoing.h"
#include "../Components/FlyingGnome.h"

void MageCustomer::Awake()
{
	AddComponent<BoingBoing>();
	AddComponent<CharacterController>();
	AddComponent<CustomerAI>().SetCustomerType(Customer::Mage);
	AddComponent<FlyingGnome>();
}
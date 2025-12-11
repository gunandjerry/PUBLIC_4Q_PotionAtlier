#pragma once
#include "Physics/PhysXFramework.h"

class ExceptActorQueryFilterCallback : public px::PxQueryFilterCallback
{
	px::PxActor* actor_to_ignore{ nullptr };

public:
	ExceptActorQueryFilterCallback(px::PxActor* actor_to_ignore) : actor_to_ignore(actor_to_ignore) {}

    virtual px::PxQueryHitType::Enum preFilter(const px::PxFilterData& filterData, const px::PxShape* shape, const px::PxRigidActor* actor, px::PxHitFlags& queryFlags) override
    {
        if (actor->userData == actor_to_ignore->userData) {
            return px::PxQueryHitType::eNONE;
        }
        return px::PxQueryHitType::eBLOCK;
    }
    virtual px::PxQueryHitType::Enum postFilter(const px::PxFilterData& filterData, const px::PxQueryHit& hit, const px::PxShape* shape, const px::PxRigidActor* actor) override
    {
        return px::PxQueryHitType::eBLOCK;
    }
};
/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "framework/EliteAI/EliteData/EBlackboard.h"

//------------
//---STATES---
//------------

namespace FSMStates
{
	class WanderState : public Elite::FSMState
	{
	public:
		WanderState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class SeekFoodState : public Elite::FSMState
	{
	public:
		SeekFoodState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
		virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;
	};

	class EvadeBorder : public Elite::FSMState
	{
	public:
		EvadeBorder() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class EvadeBiggerAgent : public Elite::FSMState
	{
	public:
		EvadeBiggerAgent() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
		virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;
	};

	class ChaseSmallerAgent : public Elite::FSMState
	{
	public:
		ChaseSmallerAgent() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
		virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;
	};
}


//-----------------
//---TRANSITIONS---
//-----------------

namespace FSMConditions
{
	class FoodNearbyCondition : public Elite::FSMCondition
	{
	public : 
		FoodNearbyCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class FoundFoodEatenCondition : public Elite::FSMCondition
	{
	public:
		FoundFoodEatenCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class BorderNearbyCondition : public Elite::FSMCondition
	{
	public:
		BorderNearbyCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class FarEnoughFromBorderCondition : public Elite::FSMCondition
	{
	public:
		FarEnoughFromBorderCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class BiggerAgentAroundCondition : public Elite::FSMCondition
	{
	public:
		BiggerAgentAroundCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class BiggerAgentFarEnoughCondition : public Elite::FSMCondition
	{
	public:
		BiggerAgentFarEnoughCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class SmallerAgentAroundCondition : public Elite::FSMCondition
	{
	public:
		SmallerAgentAroundCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class SmallerAgentEaten : public Elite::FSMCondition
	{
	public:
		SmallerAgentEaten() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};
}

#endif
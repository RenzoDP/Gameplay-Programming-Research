/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

namespace BT_Actions
{
	Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		pAgent->SetToWander();
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToSeek(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		Elite::Vector2 targetPos;
		
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Target", targetPos))
		{
			return Elite::BehaviorState::Failure;
		}

		pAgent->SetToSeek(targetPos );
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToEvade(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		AgarioAgent* pAgentToEvade;

		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("AgentFleeTarget", pAgentToEvade) || pAgentToEvade == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		// Else run
		pAgent->SetToFlee(pAgentToEvade->GetPosition());
		return Elite::BehaviorState::Success;
	}
}

namespace BT_Conditions
{
	bool IsFoodNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		std::vector<AgarioFood*>* pFoodVec;

		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("FoodVec", pFoodVec) || pFoodVec == nullptr)
		{
			return false;
		}

		const Elite::Vector2 agentPos{ pAgent->GetPosition() };

		const float searchRadius{ 50.f + pAgent->GetRadius() };

		AgarioFood* pClosestFood = nullptr;
		float closestDistanceSq{ searchRadius * searchRadius };

		for (auto& pFood : *pFoodVec)
		{
			// Get closestFood
			float distanceSq = pFood->GetPosition().DistanceSquared(agentPos);
			if (distanceSq < closestDistanceSq)
			{
				pClosestFood = pFood;
				closestDistanceSq = distanceSq;
			}
		}

		// If close food found
		if (pClosestFood != nullptr)
		{
			pBlackboard->ChangeData("Target", pClosestFood->GetPosition());
			return true;
		}

		// Else false
		return false;
	}

	bool IsSmallerGuyAround(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		std::vector<AgarioAgent*>* pAgarioAgents;

		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("AgentsVec", pAgarioAgents) || pAgarioAgents == nullptr)
		{
			return false;
		}

		const Elite::Vector2 agentPos{ pAgent->GetPosition() };
		const float searchRadius{ 25.f + pAgent->GetRadius() };

		AgarioAgent* pClosestAgent{ nullptr };
		float closestDistanceSq{ searchRadius * searchRadius };

		for (auto& pCurrentAgent : *pAgarioAgents)
		{
			// Get closestAgent
			const float distanceSq = pCurrentAgent->GetPosition().DistanceSquared(agentPos);
			
			// Also get smallest
			const bool isCloser{ distanceSq < closestDistanceSq };
			const bool isSmaller{ pCurrentAgent->GetRadius() < pAgent->GetRadius() - 1};
			if (isCloser && isSmaller)
			{
				pClosestAgent = pCurrentAgent;
				closestDistanceSq = distanceSq;
			}
		}

		// If close agent found
		if (pClosestAgent != nullptr)
		{
			pBlackboard->ChangeData("Target", pClosestAgent->GetPosition());
			return true;
		}

		// Else false
		return false;
	}

	bool IsBiggerGuyAround(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		std::vector<AgarioAgent*>* pAgarioAgents;

		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("AgentsVec", pAgarioAgents) || pAgarioAgents == nullptr)
		{
			return false;
		}

		const Elite::Vector2 agentPos{ pAgent->GetPosition() };
		const float searchRadius{ 30.f + pAgent->GetRadius() };

		AgarioAgent* pClosestAgent{ nullptr };
		float closestDistanceSq{ searchRadius * searchRadius };

		for (auto& pCurrentAgent : *pAgarioAgents)
		{
			// Get closestAgent
			const float distanceSq = pCurrentAgent->GetPosition().DistanceSquared(agentPos);

			// Also get biggest
			const bool isCloser{ distanceSq < closestDistanceSq };
			const bool isBigger{ pCurrentAgent->GetRadius() > pAgent->GetRadius() };
			if (isCloser && isBigger)
			{
				pClosestAgent = pCurrentAgent;
				closestDistanceSq = distanceSq;
			}
		}

		// If close agent found
		if (pClosestAgent != nullptr)
		{
			pBlackboard->ChangeData("AgentFleeTarget", pClosestAgent);
			return true;
		}

		// Else false
		return false;
	}

}







#endif
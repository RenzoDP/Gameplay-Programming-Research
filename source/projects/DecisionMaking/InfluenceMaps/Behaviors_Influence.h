/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the Influence version of the Agario Game
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

namespace Influence_Actions
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

		pAgent->SetToSeek(targetPos);
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

namespace Influence_Conditions
{
	bool IsFoodNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return false;
		}
	
		Elite::InfluenceMap<InfluenceGrid>* pInfluenceGrid;
		if (!pBlackboard->GetData("InfluenceMap", pInfluenceGrid) || pInfluenceGrid == nullptr)
		{
			return false;
		}


		Elite::InfluenceNode* pAgentNode{ pInfluenceGrid->GetNodeAtWorldPos(pAgent->GetPosition()) };
		
		Elite::InfluenceNode* pLowestInfluenceNode{ pAgentNode };
		float lowestInfluence{ pAgentNode->GetInfluence() };

		// Go through all nodes connected with agentNode
		for (const auto& currentConnection : pInfluenceGrid->GetNodeConnections(pAgentNode))
		{
			Elite::InfluenceNode* pNextNode{ pInfluenceGrid->GetNode(currentConnection->GetTo()) };
			const float nextNodeInfluence{ pNextNode->GetInfluence() };

			// If nodeInfluence lower then lowest influence, this nodeInfluence is the new lowest one
			if (nextNodeInfluence < lowestInfluence)
			{
				pLowestInfluenceNode = pNextNode;
				lowestInfluence = nextNodeInfluence;
			}
		}

		// If no lower then current, return false
		if (pLowestInfluenceNode == pAgentNode)
		{
			return false;
		}
		
		// Else go to lowest
		pBlackboard->ChangeData("Target", pLowestInfluenceNode->GetPosition());
		return true;
	}
}

#endif
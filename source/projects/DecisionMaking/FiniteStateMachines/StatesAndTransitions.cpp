#include "stdafx.h"
#include "StatesAndTransitions.h"

using namespace Elite;
using namespace FSMStates;

void FSMStates::WanderState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;

	const bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
	{
		return;
	}

	pAgent->SetToWander();

	//std::cout << "I am in the wandering state" << '\n';
}

void FSMStates::SeekFoodState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
	{
		return;
	}

	AgarioFood* pFood{};
	isValid = pBlackboard->GetData("FoodNearby", pFood);

	if (isValid == false || pFood == nullptr)
	{
		return;
	}

	pAgent->SetToSeek(pFood->GetPosition());

	//std::cout << "I am in the seeking state" << '\n';
}

void FSMStates::EvadeBorder::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
	{
		return;
	}

	float trimWorldSize{};

	isValid = pBlackboard->GetData("TrimWorldSize", trimWorldSize);
	if (isValid == false || trimWorldSize <= 0.f)
	{
		return;
	}

	const Vector2 agentPos{ pAgent->GetPosition() };
	
	// X-Values
	const float distanceToLeftBorder{ (Vector2(0.f,agentPos.y) - agentPos).MagnitudeSquared() };
	const float distanceToRightBorder{ (Vector2(trimWorldSize,agentPos.y) - agentPos).MagnitudeSquared() };

	// Y-Values
	const float distanceToBottomBorder{ (Vector2(agentPos.x,0.f) - agentPos).MagnitudeSquared() };
	const float distanceToTopBorder{ (Vector2(agentPos.x,trimWorldSize) - agentPos).MagnitudeSquared() };

	// Calculate closestBorder
	Vector2 pointToFlee{};
	const float closestDistance{ std::min<float>(std::min<float>(std::min<float>(distanceToLeftBorder,distanceToRightBorder),distanceToBottomBorder),distanceToTopBorder) };
	
	if (closestDistance == distanceToLeftBorder)
	{
		pointToFlee = Vector2(0.f, agentPos.y);
	}
	else if (closestDistance == distanceToRightBorder)
	{
		pointToFlee = Vector2(trimWorldSize, agentPos.y);
	}
	else if (closestDistance == distanceToBottomBorder)
	{
		pointToFlee = Vector2(agentPos.x, 0.f);
	}
	else if (closestDistance == distanceToTopBorder)
	{
		pointToFlee = Vector2(agentPos.x, trimWorldSize);
	}

	// Flee that border
	pAgent->SetToFlee(pointToFlee);
}

void FSMStates::EvadeBiggerAgent::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
	{
		return;
	}

	AgarioAgent* pAgentToEvade;

	isValid = pBlackboard->GetData("AgentToEvade", pAgentToEvade);
	if (isValid == false || pAgentToEvade == nullptr)
	{
		return;
	}

	pAgent->SetToFlee(pAgentToEvade->GetPosition());
}

void FSMStates::ChaseSmallerAgent::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
	{
		return;
	}

	AgarioAgent* pAgentToChase;

	isValid = pBlackboard->GetData("AgentToChase", pAgentToChase);
	if (isValid == false || pAgentToChase == nullptr)
	{
		return;
	}

	pAgent->SetToSeek(pAgentToChase->GetPosition());
}

void FSMStates::SeekFoodState::Update(Elite::Blackboard* pBlackboard, float deltaTime)
{
	AgarioAgent* pAgent;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
	{
		return;
	}

	AgarioFood* pFood{};
	isValid = pBlackboard->GetData("FoodNearby", pFood);

	if (isValid == false || pFood == nullptr)
	{
		return;
	}

	// If food out of range, food was eaten
	const float distanceToFood{ (pFood->GetPosition() - pAgent->GetPosition()).MagnitudeSquared() };
	const float maxDistance{ 50.f };
	if (distanceToFood >= maxDistance * maxDistance || distanceToFood <= -(maxDistance * maxDistance))
	{
		pBlackboard->ChangeData("FoodEaten", true);
	}

	// Adjust steeringBehavior
	pAgent->GetSteeringBehavior()->SetTarget(pFood->GetPosition());
	pAgent->GetSteeringBehavior()->CalculateSteering(deltaTime, pAgent);
}

void FSMStates::EvadeBiggerAgent::Update(Elite::Blackboard* pBlackboard, float deltaTime)
{
	AgarioAgent* pAgent;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
	{
		return;
	}

	AgarioAgent* pAgentToEvade;
	isValid = pBlackboard->GetData("AgentToEvade", pAgentToEvade);

	if (isValid == false || pAgentToEvade == nullptr)
	{
		return;
	}

	// Adjust steeringBehavior
	pAgent->GetSteeringBehavior()->SetTarget(pAgentToEvade->GetPosition());
	pAgent->GetSteeringBehavior()->CalculateSteering(deltaTime, pAgent);
}

void FSMStates::ChaseSmallerAgent::Update(Elite::Blackboard* pBlackboard, float deltaTime)
{
	AgarioAgent* pAgent;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
	{
		return;
	}

	AgarioAgent* pAgentToChase;
	isValid = pBlackboard->GetData("AgentToChase", pAgentToChase);

	if (isValid == false || pAgentToChase == nullptr)
	{
		return;
	}

	// Adjust steeringBehavior
	pAgent->GetSteeringBehavior()->SetTarget(pAgentToChase->GetPosition());
	pAgent->GetSteeringBehavior()->CalculateSteering(deltaTime, pAgent);
}



bool FSMConditions::FoodNearbyCondition::Evaluate(Elite::Blackboard* pBlackboard) const 
{
	AgarioAgent* pAgent;
	std::vector<AgarioFood*>* pFoodVec;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
	{
		return false;
	}

	isValid = pBlackboard->GetData("FoodVec", pFoodVec);
	if (isValid == false || pFoodVec == nullptr)
	{
		return false;
	}

	const float foodRadius{ 50.f };
	const Vector2 agentPos{ pAgent->GetPosition() };

	// Get all the food
	const auto elementDistance = [agentPos](AgarioFood* pFood1, AgarioFood* pFood2)
	{
		const float firstDistance = agentPos.DistanceSquared(pFood1->GetPosition());
		const float secondDistance = agentPos.DistanceSquared(pFood2->GetPosition());
		
		return firstDistance < secondDistance;
	};

	// Search for the closest food
	const auto closestFoodIt{ std::min_element(pFoodVec->begin(), pFoodVec->end(),elementDistance) };
	if (closestFoodIt != pFoodVec->end())
	{
		AgarioFood* pFood{ *closestFoodIt };

		// If food closes enough, go to
		if (agentPos.DistanceSquared(pFood->GetPosition()) < foodRadius * foodRadius)
		{
			pBlackboard->ChangeData("FoodNearby", pFood);
			return true;
		}
	}

	return false;
}

bool FSMConditions::FoundFoodEatenCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
	{
		return false;
	}

	AgarioFood* pFood{};
	isValid = pBlackboard->GetData("FoodNearby", pFood);

	if (isValid == false || pFood == nullptr)
	{
		return false;
	}

	bool foodEaten{ false };
	isValid = pBlackboard->GetData("FoodEaten", foodEaten);

	if (isValid == false || foodEaten == false)
	{
		return false;
	}

	// Reset foodVariables
	pBlackboard->ChangeData("FoodNearby", static_cast<AgarioFood*>(nullptr));
	pBlackboard->ChangeData("FoodEaten", false);
	return true;
}

bool FSMConditions::BorderNearbyCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
	{
		return false;
	}

	float trimWorldSize{};

	isValid = pBlackboard->GetData("TrimWorldSize", trimWorldSize);
	if (isValid == false || trimWorldSize <= 0.f)
	{
		return false;
	}

	// Check if agent too close to border
	const Vector2 agentPos{ pAgent->GetPosition() };
	const float agentRadius{ pAgent->GetRadius() };

	const float maxDistanceToBorder{ 10.f };

	// X-Values
	const float distanceToLeftBorder{ (Vector2(0.f,agentPos.y) - Vector2(agentPos.x - agentRadius,agentPos.y)).MagnitudeSquared() };
	if (distanceToLeftBorder <= maxDistanceToBorder * maxDistanceToBorder)
	{
		return true;
	}

	const float distanceToRightBorder{ (Vector2(trimWorldSize,agentPos.y) - Vector2(agentPos.x + agentRadius, agentPos.y)).MagnitudeSquared() };
	if (distanceToRightBorder <= maxDistanceToBorder * maxDistanceToBorder)
	{
		return true;
	}

	// Y-Values
	const float distanceToBottomBorder{ (Vector2(agentPos.x,0.f) - Vector2(agentPos.x,agentPos.y - agentRadius)).MagnitudeSquared() };
	if (distanceToBottomBorder <= maxDistanceToBorder * maxDistanceToBorder)
	{
		return true;
	}

	const float distanceToTopBorder{ (Vector2(agentPos.x,trimWorldSize) - Vector2(agentPos.x,agentPos.y + agentRadius)).MagnitudeSquared() };
	if (distanceToTopBorder <= maxDistanceToBorder * maxDistanceToBorder)
	{
		return true;
	}

	return false;
}

bool FSMConditions::FarEnoughFromBorderCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
	{
		return false;
	}

	float trimWorldSize{};

	isValid = pBlackboard->GetData("TrimWorldSize", trimWorldSize);
	if (isValid == false || trimWorldSize <= 0.f)
	{
		return false;
	}

	// Check if agent too close to border
	const Vector2 agentPos{ pAgent->GetPosition() };
	const float agentRadius{ pAgent->GetRadius() };

	const float maxDistanceToBorder{ 20.f };

	// X-Values
	const float distanceToLeftBorder{ (Vector2(0.f,agentPos.y) - Vector2(agentPos.x - agentRadius,agentPos.y)).MagnitudeSquared() };
	if (distanceToLeftBorder >= maxDistanceToBorder * maxDistanceToBorder)
	{
		return true;
	}

	const float distanceToRightBorder{ (Vector2(trimWorldSize,agentPos.y) - Vector2(agentPos.x + agentRadius, agentPos.y)).MagnitudeSquared() };
	if (distanceToRightBorder >= maxDistanceToBorder * maxDistanceToBorder)
	{
		return true;
	}

	// Y-Values
	const float distanceToBottomBorder{ (Vector2(agentPos.x,0.f) - Vector2(agentPos.x,agentPos.y - agentRadius)).MagnitudeSquared() };
	if (distanceToBottomBorder >= maxDistanceToBorder * maxDistanceToBorder)
	{
		return true;
	}

	const float distanceToTopBorder{ (Vector2(agentPos.x,trimWorldSize) - Vector2(agentPos.x,agentPos.y + agentRadius)).MagnitudeSquared() };
	if (distanceToTopBorder >= maxDistanceToBorder * maxDistanceToBorder)
	{
		return true;
	}

	return false;
}

bool FSMConditions::BiggerAgentAroundCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
	{
		return false;
	}

	std::vector<AgarioAgent*>* pAgentsVector;

	isValid = pBlackboard->GetData("AgentsVec", pAgentsVector);
	if (isValid == false || pAgentsVector == nullptr)
	{
		return false;
	}

	const Vector2 agentPos{ pAgent->GetPosition() };
	const float agentRadius{ pAgent->GetRadius() };
	const float maxDistance{ 25.f + agentRadius };

	for (const auto& agent : *pAgentsVector)
	{
		const float distanceFromOtherAgent{ (agent->GetPosition() - agentPos).MagnitudeSquared() };
		
		// If other agent too close && is bigger, evade that one
		const bool otherAgentTooClose{ distanceFromOtherAgent <= maxDistance * maxDistance };
		const bool otherAgentBigger{ agent->GetRadius() > agentRadius };
		if (otherAgentTooClose && otherAgentBigger)
		{
			pBlackboard->ChangeData("AgentToEvade", agent);
			return true;
		}
	}

	return false;
}

bool FSMConditions::BiggerAgentFarEnoughCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
	{
		return false;
	}

	AgarioAgent* pAgentToEvade;
	isValid = pBlackboard->GetData("AgentToEvade", pAgentToEvade);

	if (isValid == false || pAgentToEvade == nullptr)
	{
		return false;
	}

	const Vector2 agentPos{ pAgent->GetPosition() };
	const float agentRadius{ pAgent->GetRadius() };
	const float maxDistance{ 35.f + agentRadius };

	// Other agent far enough
	const float distanceToBiggerAgent{ (pAgentToEvade->GetPosition() - agentPos).MagnitudeSquared() };
	if (distanceToBiggerAgent > maxDistance * maxDistance)
	{
		pBlackboard->ChangeData("AgentToEvade", static_cast<AgarioAgent*>(nullptr));
		return true;
	}

	return false;
}

bool FSMConditions::SmallerAgentAroundCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
	{
		return false;
	}

	std::vector<AgarioAgent*>* pAgentsVector;

	isValid = pBlackboard->GetData("AgentsVec", pAgentsVector);
	if (isValid == false || pAgentsVector == nullptr)
	{
		return false;
	}

	const Vector2 agentPos{ pAgent->GetPosition() };
	const float agentRadius{ pAgent->GetRadius() };
	const float maxDistance{ 25.f + agentRadius };

	for (const auto& agent : *pAgentsVector)
	{
		const float distanceFromOtherAgent{ (agent->GetPosition() - agentPos).MagnitudeSquared() };

		// If other agent close && is smaller, chase that one
		const bool otherAgentClose{ distanceFromOtherAgent <= maxDistance * maxDistance };
		const bool otherAgentSmaller{ agent->GetRadius() < agentRadius };
		if (otherAgentClose && otherAgentSmaller)
		{
			pBlackboard->ChangeData("AgentToChase", agent);
			return true;
		}
	}

	return false;
}

bool FSMConditions::SmallerAgentEaten::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
	{
		return false;
	}

	AgarioAgent* pAgentToChase;
	isValid = pBlackboard->GetData("AgentToChase", pAgentToChase);

	if (isValid == false || pAgentToChase == nullptr)
	{
		return false;
	}

	const Vector2 agentPos{ pAgent->GetPosition() };
	const float agentRadius{ pAgent->GetRadius() };
	const float maxDistance{ 35.f + agentRadius };

	// Other agent eaten
	const float distanceToSmallerAgent{ (pAgentToChase->GetPosition() - agentPos).MagnitudeSquared() };
	if (distanceToSmallerAgent >= maxDistance * maxDistance || distanceToSmallerAgent <= -(maxDistance * maxDistance))
	{
		pBlackboard->ChangeData("AgentToChase", static_cast<AgarioAgent*>(nullptr));
		return true;
	}

	return false;
}
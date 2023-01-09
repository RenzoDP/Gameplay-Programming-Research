#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	// Agent variables
	const Elite::Vector2 agentPos{ pAgent->GetPosition() };
	const Elite::Vector2 agentVelocity{ pAgent->GetLinearVelocity() };

	const std::vector<SteeringAgent*> neighbors{ m_pFlock->GetNeighbors() };
	const int neighborCount{ m_pFlock->GetNrOfNeighbors() };

	const Elite::Vector2 averageNeighborPos{ m_pFlock->GetAverageNeighborPos() };

	Elite::Vector2 agentToTarget{ averageNeighborPos - agentPos };
	agentToTarget.Normalize();

	steering.LinearVelocity = agentToTarget;
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(agentPos, agentVelocity, 5, { 0,1,0 });
		DEBUGRENDERER2D->DrawPoint(averageNeighborPos, 5, { 0,0,1 });
		DEBUGRENDERER2D->DrawCircle(agentPos, 10.f, { 0,0,1 }, 0.9f);
	}

	return steering;
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	// Agent variables
	const Elite::Vector2 agentPos{ pAgent->GetPosition() };
	const Elite::Vector2 agentVelocity{ pAgent->GetLinearVelocity() };

	std::vector<SteeringAgent*> neighbors{ m_pFlock->GetNeighbors() };
	const int neighborCount{ m_pFlock->GetNrOfNeighbors() };
	
	std::vector<Elite::Vector2> neighborPos{};

	Elite::Vector2 closestPos{};
	float closestDistance{};

	Elite::Vector2 currNeighborPos{};
	float currNeighborDistance{};

	// Collect positions of all neighbors
	for (int idx{}; idx < neighborCount; idx++)
	{
		neighborPos.push_back(neighbors[idx]->GetPosition());
	}
	
	Elite::Vector2 desiredDirection{};
	Elite::Vector2 currNeighborDesiredDirection{};

	for (size_t idx{}; idx < neighborPos.size(); idx++)
	{
		currNeighborDesiredDirection = agentPos - neighborPos[idx];
		currNeighborDistance = currNeighborDesiredDirection.Magnitude();

		currNeighborDesiredDirection.Normalize();

		desiredDirection += (1 / currNeighborDistance) * currNeighborDesiredDirection;
	}

	desiredDirection.Normalize();
	steering.LinearVelocity = desiredDirection;
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(agentPos, agentVelocity, 5, { 0,1,0 });
		DEBUGRENDERER2D->DrawDirection(agentPos, desiredDirection, 5, { 1,0,0 });
		DEBUGRENDERER2D->DrawCircle(agentPos, 10.f, { 0,0,1 }, 0.9f);
	}

	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	// Agent variables
	const Elite::Vector2 agentPos{ pAgent->GetPosition() };
	const Elite::Vector2 agentVelocity{ pAgent->GetLinearVelocity() };

	const Elite::Vector2 averageNeighborVelocity{ m_pFlock->GetAverageNeighborVelocity() };

	Elite::Vector2 desiredDirection{ averageNeighborVelocity - agentVelocity };
	desiredDirection.Normalize();

	steering.LinearVelocity = desiredDirection;
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(agentPos, agentVelocity, 5, { 0,1,0 });
		DEBUGRENDERER2D->DrawDirection(agentPos, desiredDirection, 5, { 1,0,0 });
		DEBUGRENDERER2D->DrawCircle(agentPos, 10.f, { 0,0,1 }, 0.9f);
	}

	return steering;
}
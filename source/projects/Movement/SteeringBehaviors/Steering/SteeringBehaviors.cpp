//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
	}

	return steering;
}

//FLEE
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= -pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
	}

	return steering;
}

//ARRIVE
//****
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	float distance{};
	const float slowRadius = 15.f;

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	distance = steering.LinearVelocity.MagnitudeSquared();
	steering.LinearVelocity.Normalize();

	if (distance < slowRadius * slowRadius)
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * distance / (slowRadius * slowRadius);
	}
	else
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}
	

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
	}

	return steering;
}

//FACE
//****
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	pAgent->SetAutoOrient(false);

	Elite::Vector2 toVector{ m_Target.Position - pAgent->GetPosition() };
	toVector.Normalize();

	// Vector of current lookingDirection
	Elite::Vector2 lookingVector{ std::cosf(pAgent->GetRotation()),std::sinf(pAgent->GetRotation()) };
	lookingVector.Normalize();

	const float angleBetween{ Elite::AngleBetween(toVector, lookingVector)};

	steering.AngularVelocity = -angleBetween;
	steering.AngularVelocity *= pAgent->GetMaxAngularSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), lookingVector, 5, {0,1,0});
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), toVector, 5, { 1,0,0 });
	}

	return steering;
}

//WANDER
//****
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	// Agent variables
	const Elite::Vector2 agentPos{ pAgent->GetPosition() };
	const Elite::Vector2 directionVector{ pAgent->GetDirection() };
	Elite::Vector2 lookingVector{ std::cosf(pAgent->GetRotation()),std::sinf(pAgent->GetRotation()) };
	lookingVector.Normalize();

	// Circle variables
	const float gapDistance{ 0.5f };
	const Elite::Vector2 circlePosition{ agentPos + lookingVector * (gapDistance + m_Radius) };

	float offSetX{}, offSetY{};
	Elite::Vector2 randomPos{};

	// After time-frame give random pos within circle
	m_TimePassed += deltaT;
	if (m_TimePassed >= 1.f)
	{
		offSetX = (rand() % int((2 * (m_Radius - 1)) * 100.f)) / 100.f - m_Radius + 1;
		offSetY = (rand() % int((2 * (m_Radius - 1)) * 100.f)) / 100.f - m_Radius + 1;

		randomPos.x = offSetX + circlePosition.x;
		randomPos.y = offSetY + circlePosition.y;

		// Vector from circlePos to randomPos in circle
		m_RandomDirection = randomPos - circlePosition;

		m_TimePassed = 0;
	}

	Elite::Vector2 guidedVector{ m_RandomDirection };
	guidedVector.Normalize();

	// Project to randomPoint onto the circle
	m_ProjectedPoint = circlePosition + m_Radius * guidedVector;

	// Make agent move towards projectedPoint
	Elite::Vector2 toVector{ m_ProjectedPoint - agentPos };
	toVector.Normalize();

	steering.LinearVelocity = toVector;
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(agentPos, directionVector, 5, { 1,0,0 });
		DEBUGRENDERER2D->DrawCircle(circlePosition, m_Radius, { 0,0,1 }, 0.9f);
		DEBUGRENDERER2D->DrawPoint(m_ProjectedPoint, 5.f, { 0,0,1 });
		DEBUGRENDERER2D->DrawDirection(agentPos, toVector, 5, { 0,1,0 });
	}

	return steering;
}

//Pursuit
//****
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	// Agent variables
	const Elite::Vector2 agentPos{ pAgent->GetPosition() };
	const Elite::Vector2 agentVelocity{ pAgent->GetLinearVelocity() };
	Elite::Vector2 lookingVector{ std::cosf(pAgent->GetRotation()),std::sinf(pAgent->GetRotation()) };
	lookingVector.Normalize();

	// Target Velocity
	const Elite::Vector2 targetVelocity{ m_Target.LinearVelocity };
	
	// Point where target is headed
	const Elite::Vector2 desiredPoint{ targetVelocity + m_Target.Position };
	const Elite::Vector2 desiredVelocity{ desiredPoint - agentPos };

	// The adjusting velocity that agent needs to get there
	Elite::Vector2 adjustVelocity{ desiredVelocity - agentVelocity };
	adjustVelocity.Normalize();

	steering.LinearVelocity = adjustVelocity;
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(agentPos, agentVelocity, 5, { 0,1,0 });
		DEBUGRENDERER2D->DrawDirection(m_Target.Position, targetVelocity, 5, {1,0,0});
		DEBUGRENDERER2D->DrawDirection(agentPos, adjustVelocity, 5, { 1,0,0 });
		DEBUGRENDERER2D->DrawPoint(desiredPoint, 5.f, { 0,0,1 });
	}

	return steering;
}

//EVADE
//****
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	// Radius
	Elite::Vector2 toTarget = pAgent->GetPosition() - (m_Target).Position;
	float distanceSquared = toTarget.MagnitudeSquared();

	if (distanceSquared > m_EvadeRadius * m_EvadeRadius)
	{
		steering.IsValid = false;
		return steering;
	}

	// Agent variables
	const Elite::Vector2 agentPos{ pAgent->GetPosition() };
	const Elite::Vector2 agentVelocity{ pAgent->GetLinearVelocity() };
	Elite::Vector2 lookingVector{ std::cosf(pAgent->GetRotation()),std::sinf(pAgent->GetRotation()) };
	lookingVector.Normalize();

	// Target Velocity
	const Elite::Vector2 targetVelocity{ m_Target.LinearVelocity };

	// Point where target is headed
	const Elite::Vector2 desiredPoint{ targetVelocity + m_Target.Position };
	const Elite::Vector2 desiredVelocity{ desiredPoint - agentPos };

	// The adjusting velocity that agent needs to get there
	Elite::Vector2 adjustVelocity{ desiredVelocity - agentVelocity };
	adjustVelocity.Normalize();

	steering.LinearVelocity = -adjustVelocity;
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(agentPos, agentVelocity, 5, { 0,1,0 });
		DEBUGRENDERER2D->DrawDirection(m_Target.Position, targetVelocity, 5, { 1,0,0 });
		DEBUGRENDERER2D->DrawDirection(agentPos, adjustVelocity, 5, { 1,0,0 });
		DEBUGRENDERER2D->DrawPoint(desiredPoint, 5.f, { 0,0,1 });
		DEBUGRENDERER2D->DrawCircle(agentPos, m_EvadeRadius, { 0,0,1 }, 0.9f);
	}

	return steering;
}

//STANDSTILL
//**********
SteeringOutput StandStill::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	return steering;
}
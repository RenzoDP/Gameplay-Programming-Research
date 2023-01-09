#include "stdafx.h"
#include "AgentManager.h"

AgentManager::AgentManager(const std::vector<Elite::Vector2>& worldMaxima)
{
	// Init Agents
	InitializeAgents();
}

AgentManager::~AgentManager()
{
	// Delete guardAgent
	SAFE_DELETE(m_pGuardAgent);

	SAFE_DELETE(m_pGuardSeekingBehavior);
	SAFE_DELETE(m_pGuardStandStill);

	// Delete playerAgent
	SAFE_DELETE(m_pPlayerAgent);

	SAFE_DELETE(m_pPlayerArriveBehavior);
	SAFE_DELETE(m_pPlayerSeekingBehavior);
}

void AgentManager::Update(float deltaTime)
{
	// Update Player
	UpdatePlayer(deltaTime);

	// Update Guard
	UpdateGuard(deltaTime);
}

void AgentManager::Render(float deltaTime)
{
	// Render MousePos
	DEBUGRENDERER2D->DrawPoint(m_PlayerTarget, 5.f, { 1,0,0 }, 0.09f);

	// Render guard Vision Cone
	Elite::Polygon guardConePolygon{ {m_pGuardAgent->GetPosition(), m_LeftConePoint, m_RightConePoint} };
	DEBUGRENDERER2D->DrawPolygon(&guardConePolygon, { 1,1,1 }, 0.09f);

	// Render agents
	m_pPlayerAgent->Render(deltaTime);
	m_pGuardAgent->Render(deltaTime);
}

void AgentManager::SetTarget(const Elite::Vector2& newTarget)
{
	m_PlayerTarget = newTarget;
}

bool AgentManager::GetHasLostPlayer() const
{
	return m_HasLostPlayer;
}
Elite::Vector2 AgentManager::GetPlayerPos() const
{
	return m_pPlayerAgent->GetPosition();
}
Elite::Vector2 AgentManager::GetPlayerVelocity() const
{
	return m_pPlayerAgent->GetLinearVelocity();
}

void AgentManager::InitializeAgents()
{
	// Init Agents
	// ===========

	Elite::Vector2 startPos{};

	// Init Player Agent
	// =================
	m_pPlayerSeekingBehavior = new Seek();
	m_pPlayerArriveBehavior = new Arrive();

	m_pPlayerAgent = new SteeringAgent();
	m_pPlayerAgent->SetSteeringBehavior(m_pPlayerSeekingBehavior);
	m_pPlayerAgent->SetMaxLinearSpeed(m_MaxPlayerLinearSpeed);
	m_pPlayerAgent->SetAutoOrient(true);
	m_pPlayerAgent->SetMass(0.1f);
	m_pPlayerAgent->SetBodyColor(Elite::Color{ 0,0,1.f });

	startPos = Elite::Vector2{ 10.f, 40.f };
	m_pPlayerAgent->SetPosition(startPos);
	m_PlayerTarget = startPos;

	// Init Guard Agent
	// ================
	m_pGuardSeekingBehavior = new Seek();
	m_pGuardStandStill = new StandStill();

	m_pGuardAgent = new SteeringAgent();
	m_pGuardAgent->SetMaxLinearSpeed(m_MaxGuardLinearSpeed);
	m_pGuardAgent->SetSteeringBehavior(m_pGuardStandStill);
	m_pGuardAgent->SetAutoOrient(true);
	m_pGuardAgent->SetMass(0.1f);
	m_pGuardAgent->SetBodyColor(m_GuardNeutralColor);

	startPos = Elite::Vector2{ 20.f, 10.f };
	m_pGuardAgent->SetPosition(startPos);
	m_pGuardAgent->SetRotation(Elite::ToRadians(90.f));

	// Init Cone
	// =========
	const float guardRotation{ m_pGuardAgent->GetRotation() };
	const Elite::Vector2 guardLookingVector{ std::cosf(guardRotation),std::sinf(guardRotation) };

	Elite::Vector2 leftConePointAdjustments{ -7.5f, 15.f };
	m_ConeLength = leftConePointAdjustments.Magnitude();
	leftConePointAdjustments.Normalize();

	Elite::Vector2 rightConePointAdjustments{ 7.5f, 15.f };
	rightConePointAdjustments.Normalize();

	m_LeftConeAngleDifference = AngleBetween(guardLookingVector, leftConePointAdjustments);
	m_RightConeAngleDifference = AngleBetween(guardLookingVector, rightConePointAdjustments);

	m_LeftConePoint = startPos + m_ConeLength * Elite::Vector2{ std::cosf(guardRotation + m_LeftConeAngleDifference), std::sinf(guardRotation + m_LeftConeAngleDifference) };
	m_RightConePoint = startPos + m_ConeLength * Elite::Vector2{ std::cosf(guardRotation + m_RightConeAngleDifference), std::sinf(guardRotation + m_RightConeAngleDifference) };
}

void AgentManager::UpdatePlayer(float deltaTime)
{
	// Player Agent
	// ============

	//Check if agent is close to Target
	const Elite::Vector2 agentPos{ m_pPlayerAgent->GetPosition() };

	const float minDistance{ 3.f };
	const float distanceToPoint{ (m_PlayerTarget - agentPos).MagnitudeSquared() };

	// Change Behavior when necessary
	const bool isCloserThenMinDistance{ distanceToPoint <= minDistance * minDistance };
	if (isCloserThenMinDistance)
	{
		m_pPlayerAgent->SetSteeringBehavior(m_pPlayerArriveBehavior);
		m_pPlayerArriveBehavior->SetTarget(m_PlayerTarget);
	}
	else
	{
		m_pPlayerAgent->SetSteeringBehavior(m_pPlayerSeekingBehavior);
		m_pPlayerSeekingBehavior->SetTarget(m_PlayerTarget);
	}

	// Update Agent
	m_pPlayerAgent->Update(deltaTime);
}
void AgentManager::UpdateGuard(float deltaTime)
{
	// Guard Agent
	// ===========

	// Adjust visionCone
	// -----------------
	const Elite::Vector2 guardPos{ m_pGuardAgent->GetPosition() };

	const float guardRotation{ m_pGuardAgent->GetRotation() };
	const Elite::Vector2 guardLookingVector{ std::cosf(guardRotation),std::sinf(guardRotation) };

	m_LeftConePoint = guardPos + m_ConeLength * Elite::Vector2{ std::cosf(guardRotation + m_LeftConeAngleDifference), std::sinf(guardRotation + m_LeftConeAngleDifference) };
	m_RightConePoint = guardPos + m_ConeLength * Elite::Vector2{ std::cosf(guardRotation + m_RightConeAngleDifference), std::sinf(guardRotation + m_RightConeAngleDifference) };

	// Adjust steering
	// ---------------
	m_pGuardSeekingBehavior->SetTarget(m_pPlayerAgent->GetPosition());

	// If Player visible, follow Player
	const bool playerInCone{ Elite::PointInTriangle(m_pPlayerAgent->GetPosition(), guardPos, m_LeftConePoint, m_RightConePoint) };
	if (playerInCone)
	{
		// Change Guard Variables
		m_pGuardAgent->SetSteeringBehavior(m_pGuardSeekingBehavior);
		m_pGuardAgent->SetAutoOrient(true);

		m_pGuardAgent->SetBodyColor(m_GuardChasingColor);

		// Change Bools
		m_HasSeenPlayerAlready = true;
		m_HasLostPlayer = false;
	}
	// If lost Player, use InfluenceMap
	else if (m_HasSeenPlayerAlready)
	{
		// Change Guard Variables
		m_pGuardAgent->SetBodyColor(m_GuardAlertColor);

		// Change Bools
		m_HasLostPlayer = true;
	}
	// Else, just stand Still
	else
	{
		// Change Guard Variables
		m_pGuardAgent->SetSteeringBehavior(m_pGuardStandStill);
		m_pGuardAgent->SetAutoOrient(false);

		m_pGuardAgent->SetBodyColor(m_GuardNeutralColor);

		// Change Bools
		m_HasLostPlayer = false;
	}

	// Update Guard
	// ------------
	m_pGuardAgent->Update(deltaTime);
}
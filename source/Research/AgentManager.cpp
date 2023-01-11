#include "stdafx.h"
#include "AgentManager.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAstar.h"

AgentManager::AgentManager(Elite::InfluenceMap<InfluenceGrid>* pInfluenceGrid, const std::vector<Elite::Vector2>& worldMaxima)
{
	// Init Agents
	InitializeAgents(worldMaxima);

	// Init influenceGrid
	m_pInfluenceGrid = pInfluenceGrid;
}

AgentManager::~AgentManager()
{
	// Delete guardAgent
	SAFE_DELETE(m_pGuardAgent);

	SAFE_DELETE(m_pGuardSeekingBehavior);
	SAFE_DELETE(m_pStandStillBehavior);

	// Delete playerAgent
	SAFE_DELETE(m_pPlayerAgent);

	SAFE_DELETE(m_pPlayerArriveBehavior);
	SAFE_DELETE(m_pPlayerSeekingBehavior);
}

void AgentManager::Update(float deltaTime, bool algorithmIsBusy)
{
	// Update Player
	UpdatePlayer(deltaTime);

	// Update Guard
	UpdateGuard(deltaTime,algorithmIsBusy);
}

void AgentManager::Render(float deltaTime)
{
	// Render MousePos
	DEBUGRENDERER2D->DrawPoint(m_PlayerTarget, 5.f, { 1,0,0 }, 0.09f);

	// Render GuardTarget
	if (m_CurrentGuardBehavior == Chasing)
	{
		DEBUGRENDERER2D->DrawPoint(m_GuardTarget, 5.f, { 0,1,0 }, 0.09f);
	}

	// Render guard Vision Cone
	Elite::Polygon guardConePolygon{ {m_pGuardAgent->GetPosition(), m_LeftConePoint, m_RightConePoint} };
	DEBUGRENDERER2D->DrawPolygon(&guardConePolygon, { 1,1,1 }, 0.09f);

	// Render agents
	m_pPlayerAgent->Render(deltaTime);
	m_pGuardAgent->Render(deltaTime);
}

void AgentManager::SetPlayerTarget(const Elite::Vector2& newTarget)
{
	m_PlayerTarget = newTarget;

	m_pPlayerAgent->SetSteeringBehavior(m_pPlayerSeekingBehavior);
	m_pPlayerSeekingBehavior->SetTarget(newTarget);
	CalculatePath(m_pPlayerAgent->GetPosition(), newTarget, m_vPlayerPath, true);
}
void AgentManager::SetGuardTarget(const Elite::Vector2& newTarget)
{
	m_GuardTarget = newTarget;
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

AgentManager::GuardBehavior AgentManager::GetCurrentGuardBehavior() const
{
	return m_CurrentGuardBehavior;
}

void AgentManager::InitializeAgents(const std::vector<Elite::Vector2>& worldMaxima)
{
	// Init Agents
	// ===========

	Elite::Vector2 startPos{};

	// Init Player Agent
	// =================
	m_pPlayerSeekingBehavior = new Seek();
	m_pPlayerArriveBehavior = new Arrive();

	m_pPlayerAgent = new SteeringAgent();
	m_pPlayerAgent->SetSteeringBehavior(m_pStandStillBehavior);
	m_pPlayerAgent->SetMaxLinearSpeed(m_MaxPlayerLinearSpeed);
	m_pPlayerAgent->SetAutoOrient(true);
	m_pPlayerAgent->SetMass(0.1f);
	m_pPlayerAgent->SetBodyColor(Elite::Color{ 0,0,1.f });

	startPos = Elite::Vector2{ worldMaxima[1].x / 2.f - 10.f, worldMaxima[1].y / 2.f + 15.f - 20.f};
	m_pPlayerAgent->SetPosition(startPos);
	m_PlayerTarget = startPos;

	// Init Guard Agent
	// ================
	m_pGuardSeekingBehavior = new Seek();
	m_pStandStillBehavior = new StandStill();

	m_pGuardAgent = new SteeringAgent();
	m_pGuardAgent->SetMaxLinearSpeed(m_MaxGuardLinearSpeed);
	m_pGuardAgent->SetSteeringBehavior(m_pStandStillBehavior);
	m_pGuardAgent->SetAutoOrient(false);
	m_pGuardAgent->SetMass(0.1f);
	m_pGuardAgent->SetBodyColor(m_GuardNeutralColor);

	startPos = Elite::Vector2{ worldMaxima[1].x / 2.f, worldMaxima[1].y / 2.f - 20.f };
	m_pGuardAgent->SetPosition(startPos);
	m_pGuardAgent->SetRotation(Elite::ToRadians(90.f));

	// Init Cone
	// =========
	const float guardRotation{ m_pGuardAgent->GetRotation() };
	const Elite::Vector2 guardLookingVector{ std::cosf(guardRotation),std::sinf(guardRotation) };

	const float adjustedFOV{ m_FOV / 10.f };

	Elite::Vector2 leftConePointAdjustments{ -adjustedFOV, 10.f };
	leftConePointAdjustments.Normalize();

	Elite::Vector2 rightConePointAdjustments{ adjustedFOV, 10.f };
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

	// Follow Path
	// -----------
	FollowPath(m_pPlayerAgent, m_pPlayerSeekingBehavior, m_pPlayerArriveBehavior, m_vPlayerPath);

	// Update Agent
	// ------------
	m_pPlayerAgent->Update(deltaTime);
}
void AgentManager::UpdateGuard(float deltaTime, bool algorithmIsBusy)
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

	// Original Behavior
	// -----------------
	GuardBehavior originalBehavior{ m_CurrentGuardBehavior };

	// If was Chasing, but algorithm stopped, onGuard
	// ----------------------------------------------
	const bool returnToOnGuard{ algorithmIsBusy == false && m_CurrentGuardBehavior == Chasing };
	if (returnToOnGuard) m_CurrentGuardBehavior = OnGuard;

	// If Player not in cone and was Alert, Chase
	// ------------------------------------------
	const bool playerInCone{ Elite::PointInTriangle(m_pPlayerAgent->GetPosition(), guardPos, m_LeftConePoint, m_RightConePoint) };
	const bool lostPlayer{ playerInCone == false && m_CurrentGuardBehavior == Alert };
	if (lostPlayer) m_CurrentGuardBehavior = Chasing;

	// If Player visible, follow Player
	// ---------------------------------
	if (playerInCone) m_CurrentGuardBehavior = Alert;

	// Adjust steering if Behavior changed
	// -----------------------------------

	if (originalBehavior != m_CurrentGuardBehavior)
	{
		switch (m_CurrentGuardBehavior)
		{
		case AgentManager::OnGuard:
			// Change Guard Variables
			m_pGuardAgent->SetSteeringBehavior(m_pStandStillBehavior);
			m_pGuardAgent->SetAutoOrient(false);

			m_pGuardAgent->SetBodyColor(m_GuardNeutralColor);
			break;

		case AgentManager::Chasing:
			// Change Guard Variables
			//m_pGuardAgent->SetSteeringBehavior(m_pGuardSeekingBehavior);
			m_pGuardAgent->SetAutoOrient(false);

			m_pGuardAgent->SetBodyColor(m_GuardAlertColor);

			// Change Bools
			m_HasLostPlayer = true;
			m_HasSeenPlayerAlready = false;
			break;

		case AgentManager::Alert:
			// Change Guard Variables
			//m_pGuardAgent->SetSteeringBehavior(m_pGuardSeekingBehavior);
			m_pGuardAgent->SetAutoOrient(false);

			m_pGuardAgent->SetBodyColor(m_GuardChasingColor);

			// Change Bools
			m_HasSeenPlayerAlready = true;
			m_HasLostPlayer = false;
			break;
		}
	}

	if (m_CurrentGuardBehavior == Chasing) m_pGuardSeekingBehavior->SetTarget(m_GuardTarget);
	if (m_CurrentGuardBehavior == Alert) m_pGuardSeekingBehavior->SetTarget(m_pPlayerAgent->GetPosition());

	// Update Guard
	// ------------
	m_pGuardAgent->Update(deltaTime);
}

void AgentManager::CalculatePath(const Elite::Vector2& startPos, const Elite::Vector2& endPos, std::vector<Elite::InfluenceNode*>& path, bool isPlayer)
{
	// Check if start- and end-pos are valid
	// ------------------------------------
	const int startPathIdx{ m_pInfluenceGrid->GetNodeIdxAtWorldPos(startPos) };
	const int endPathIdx{ m_pInfluenceGrid->GetNodeIdxAtWorldPos(endPos) };

	const bool areValidPositions{ startPathIdx != invalid_node_index && endPathIdx != invalid_node_index && startPathIdx != endPathIdx };
	if (areValidPositions)
	{
		// A* Pathfinding
		// --------------
		const Elite::Heuristic heuristicFunction = Elite::HeuristicFunctions::Chebyshev;

		auto pathFinder = Elite::AStar<Elite::InfluenceNode, Elite::GraphConnection>(m_pInfluenceGrid, heuristicFunction);
		auto startNode = m_pInfluenceGrid->GetNode(startPathIdx);
		auto endNode = m_pInfluenceGrid->GetNode(endPathIdx);

		path = pathFinder.FindPath(startNode, endNode);
	}
	else
	{
		if (isPlayer)
		{
			m_pPlayerAgent->SetSteeringBehavior(m_pStandStillBehavior);
		}
		else
		{
			m_pGuardAgent->SetSteeringBehavior(m_pStandStillBehavior);
		}

		path.clear();
	}
}
void AgentManager::FollowPath(SteeringAgent* pAgent, ISteeringBehavior* pSeekingBehavior, ISteeringBehavior* pArriveBehavior, std::vector<Elite::InfluenceNode*>& path)
{
	//Check if a path exist and move to the following point
	// ----------------------------------------------------
	if (path.size() > 0)
	{
		const Elite::Vector2 firstPathPos{ m_pInfluenceGrid->GetNodeWorldPos(path[0]) };
		const float agentRadius{ pAgent->GetRadius() };

		if (path.size() == 1)
		{
			//We have reached the last node
			pAgent->SetSteeringBehavior(pArriveBehavior);
			pArriveBehavior->SetTarget(firstPathPos);
		}
		else
		{
			//Move to the next node
			pAgent->SetSteeringBehavior(pSeekingBehavior);
			pSeekingBehavior->SetTarget(firstPathPos);
		}

		if (Elite::DistanceSquared(pAgent->GetPosition(), firstPathPos) < agentRadius * agentRadius)
		{
			//If we reached the next point of the path. Remove it 
			path.erase(std::remove(path.begin(), path.end(), path[0]));
		}
	}
}
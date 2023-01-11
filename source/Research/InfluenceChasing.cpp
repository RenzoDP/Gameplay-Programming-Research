#include "stdafx.h"
using namespace Elite;

//Includes
#include "InfluenceChasing.h"
#include "AgentManager.h"
#include "ChasingAlgorithm.h"

InfluenceChasing::InfluenceChasing(Elite::InfluenceMap<InfluenceGrid>* pInfluenceGrid, const std::vector<Elite::Vector2>& worldMaxima,
								   int cellAmount, int cellSize, bool isDirectionalGraph, bool isConnectedDiagonally)
	: m_pInfluenceGrid{pInfluenceGrid}
{
	m_pAgentManager = new AgentManager(pInfluenceGrid, worldMaxima);

	m_pChasingAlgorithm = new ChasingAlgorithm(cellAmount, cellSize, isDirectionalGraph, isConnectedDiagonally);
}

InfluenceChasing::~InfluenceChasing()
{
	SAFE_DELETE(m_pAgentManager);
	SAFE_DELETE(m_pChasingAlgorithm);
}

void InfluenceChasing::Update(float deltaTime)
{
	// Handle Input
	HandleInput(deltaTime);

	// Handle Agents
	HandleAgents(deltaTime);

	// Handle Algorithm
	m_pChasingAlgorithm->Update(deltaTime);
}

void InfluenceChasing::Render(float deltaTime)
{
	// Render Agents
	if (m_RenderAgents)
	{
		m_pAgentManager->Render(deltaTime);
	}

	// Render Graph
	m_pChasingAlgorithm->Render(deltaTime);

	// Render Debug Mouse Clicks
	if (m_TimesClicked != 0 && m_pChasingAlgorithm->GetStartedAlgorithm() == false)
	{
		if (m_TimesClicked == 1) DEBUGRENDERER2D->DrawPoint(m_MouseClickPlayerPos, 10.f, { 1.f,0.f,1.f }, 0.09f);
	}
}

void InfluenceChasing::SetRenderAsGraph(bool renderAsGraph)
{
	m_pChasingAlgorithm->SetRenderAsGraph(renderAsGraph);
}

float InfluenceChasing::GetMaxHeat() const
{
	return m_pChasingAlgorithm->GetMaxHeat();
}
float InfluenceChasing::GetMaxAmountOfHeatedCells() const
{
	return m_pChasingAlgorithm->GetMaxAmountOfHeatedCells();
}
float InfluenceChasing::GetMaxPropagationSteps() const
{
	return m_pChasingAlgorithm->GetMaxPropagationSteps();
}
float InfluenceChasing::GetPropagationInterval() const
{
	return m_pChasingAlgorithm->GetPropagationInterval();
}

void InfluenceChasing::SetMaxHeat(float maxHeat)
{
	m_pChasingAlgorithm->SetMaxHeat(maxHeat);
}
void InfluenceChasing::SetMaxAmountOfHeatedCells(float maxAmountOfHeatedCells)
{
	m_pChasingAlgorithm->SetMaxAmountOfHeatedCells(maxAmountOfHeatedCells);
}
void InfluenceChasing::SetMaxPropagationSteps(float maxPropagationSteps)
{
	m_pChasingAlgorithm->SetMaxPropagationSteps(maxPropagationSteps);
}
void InfluenceChasing::SetPropagationInterval(float propagationInterval)
{
	m_pChasingAlgorithm->SetPropagationInterval(propagationInterval);
}

void InfluenceChasing::SetRenderAgents(bool renderAgents)
{
	m_RenderAgents = renderAgents;
}

std::vector<Elite::InfluenceNode*> InfluenceChasing::UpdateGraphNodes(Elite::GraphEditor& m_GridEditor)
{
	// Change Color of Nodes with no Connections
	// =========================================

	std::vector<Elite::InfluenceNode*> blockedNodes{};

	// Update chasingAlgorithm Grid
	m_GridEditor.UpdateGraph(m_pChasingAlgorithm->GetInfluenceGrid());
	AddBlockedNodes(m_pChasingAlgorithm->GetInfluenceGrid(), blockedNodes, true);
	m_pChasingAlgorithm->SetBlockedNodes(blockedNodes);

	// Update InfluenceGrid
	blockedNodes.clear();
	AddBlockedNodes(m_pInfluenceGrid, blockedNodes, true);

	// Return
	return blockedNodes;
}

void InfluenceChasing::HandleInput(float deltaTime)
{
	//------ INPUT ------
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft) && m_pChasingAlgorithm->GetStartedAlgorithm() == false)
	{
		++m_TimesClicked;

		// First Click
		if (m_TimesClicked == 1)
		{
			// Set PlayerPos
			m_MouseClickPlayerPos = GetMouseTarget(InputMouseButton::eLeft);
		}
		// Second Click
		else if (m_TimesClicked == 2)
		{
			// Set Player Velocity
			m_MouseClickVelocityPos = GetMouseTarget(InputMouseButton::eLeft);

			// Reset timesClicked
			m_TimesClicked = 0;

			// Start Algorithm
			const Elite::Vector2 mouseDirection{ (m_MouseClickVelocityPos - m_MouseClickPlayerPos).GetNormalized() * 15.f };
			m_pChasingAlgorithm->StartAlgorithm(m_MouseClickPlayerPos, mouseDirection);
		}
	}
		
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eRight))
	{
		SetMouseTarget(InputMouseButton::eRight);
	}
}

void InfluenceChasing::HandleAgents(float deltaTime)
{
	// Don't update if isn't rendered
	// ------------------------------
	if (m_RenderAgents == false) return;

	// If lost Player and didn't start algorithm yet, do so
	// ----------------------------------------------------
	const bool hasLostPlayer{ m_pAgentManager->GetHasLostPlayer() };
	const bool statusHasChanged{ hasLostPlayer != m_HasLostPlayer };
	const AgentManager::GuardBehavior currentGuardBehavior{ m_pAgentManager->GetCurrentGuardBehavior() };

	if (currentGuardBehavior == AgentManager::Chasing && statusHasChanged)
	{
		// Start Algorithm
		m_pChasingAlgorithm->StartAlgorithm(m_pAgentManager->GetPlayerPos(), m_pAgentManager->GetPlayerVelocity());
	}
	// If Player was found, reset
	else if(currentGuardBehavior == AgentManager::OnGuard && statusHasChanged)
	{
		// Stop Algorithm
		m_pChasingAlgorithm->StopAlgorithm();
	}

	m_HasLostPlayer = hasLostPlayer;

	// Calculate GuardTarget if started
	// --------------------------------

	if (m_pChasingAlgorithm->GetStartedAlgorithm())
	{
		// Get all Nodes
		const auto pInfluenceGrid{ m_pChasingAlgorithm->GetInfluenceGrid() };
		const auto allNodes{ pInfluenceGrid->GetAllNodes() };

		// Weighter Average Position
		Elite::Vector2 weightedAveragePosition{};
		std::vector<Elite::Vector2> heatedNodesPos{};

		// Loop through all nodes
		for (const auto& currentNode : allNodes)
		{
			// Heat counts as Weight
			const float currentHeat{ currentNode->GetInfluence() };

			if (currentHeat != 0.f)
			{
				Elite::Vector2 currentPosition{ pInfluenceGrid->GetNodeWorldPos(currentNode) };
				weightedAveragePosition += currentHeat * currentPosition;

				heatedNodesPos.push_back(currentPosition);
			}
		}

		// Calculate Average
		weightedAveragePosition /= static_cast<float>(heatedNodesPos.size());

		// Set Pos to nearest heatedNode, if current averagePos isn't valid
		const bool averagePosNotValid{ m_pInfluenceGrid->GetNodeIdxAtWorldPos(weightedAveragePosition) == invalid_node_index };
		if (averagePosNotValid)
		{
			float closestDistance{ FLT_MAX };
			Elite::Vector2 closestPos{};

			for (const auto& currentPos : heatedNodesPos)
			{
				// If currentDistance is closer, is new closest
				const float currentDistance{ (currentPos - weightedAveragePosition).MagnitudeSquared() };
				if (currentDistance < closestDistance)
				{
					closestDistance = currentDistance;
					closestPos = currentPos;
				}
			}

			// Set pos to closest heatedCell
			weightedAveragePosition = closestPos;
		}
	

		// Set guardTarget
		m_pAgentManager->SetGuardTarget(weightedAveragePosition);
	}

	// Update Manager
	// --------------
	m_pAgentManager->Update(deltaTime, m_pChasingAlgorithm->GetStartedAlgorithm());
}

void InfluenceChasing::SetMouseTarget(Elite::InputMouseButton mouseBtn)
{
	auto mouseData = INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, mouseBtn);
	auto mousePos = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld(Vector2{ (float)mouseData.X, (float)mouseData.Y });

	m_pAgentManager->SetPlayerTarget(mousePos);
}
Elite::Vector2 InfluenceChasing::GetMouseTarget(Elite::InputMouseButton mouseBtn) const
{
	auto mouseData = INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, mouseBtn);
	auto mousePos = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld(Vector2{ (float)mouseData.X, (float)mouseData.Y });

	return mousePos;
}

void InfluenceChasing::AddBlockedNodes(Elite::InfluenceMap<InfluenceGrid>* pInfluenceGrid, std::vector<Elite::InfluenceNode*>& blockedNodes, bool storeInBlockedNodes)
{
	const Elite::Color blockedNodeColor{ 1.f,1.f,1.f };

	// Go through all the Nodes
	for (auto& currentNode : pInfluenceGrid->GetAllNodes())
	{
		const auto nodeConnections{ pInfluenceGrid->GetNodeConnections(currentNode) };

		// If no Connections
		const bool hasNoConnections{ nodeConnections.size() == 0 };
		if (hasNoConnections)
		{
			// Set Color white and put in blockedNodes
			currentNode->SetColor(blockedNodeColor);

			// Store, when asked to
			if (storeInBlockedNodes) blockedNodes.push_back(currentNode);
		}
	}
}
#include "stdafx.h"
using namespace Elite;

//Includes
#include "InfluenceChasing.h"
#include "AgentManager.h"

InfluenceChasing::InfluenceChasing(Elite::InfluenceMap<InfluenceGrid>* pInfluenceGrid, const std::vector<Elite::Vector2>& worldMaxima)
	: m_pInfluenceGrid{pInfluenceGrid}
{
	m_pAgentManager = new AgentManager(worldMaxima);
}

InfluenceChasing::~InfluenceChasing()
{
	SAFE_DELETE(m_pAgentManager);
}

void InfluenceChasing::Update(float deltaTime)
{
	// Handle Input
	HandleInput(deltaTime);

	// Handle Agents
	HandleAgents(deltaTime);
}

void InfluenceChasing::Render(float deltaTime)
{
	// Render Agents
	m_pAgentManager->Render(deltaTime);

	// Render Algorithm
	if (m_HasStartedAlgorithm)
	{
		DEBUGRENDERER2D->DrawPoint(m_LastKnownPlayerPos, 10.f, { 1.f,0.f,1.f }, 0.09f);
		DEBUGRENDERER2D->DrawSegment(m_LastKnownPlayerPos, m_LastKnownPlayerPos + m_LastKnownPlayerVelocity, { 1.f,1.f,1.f }, 0.091f);
	}
}

std::vector<Elite::InfluenceNode*> InfluenceChasing::UpdateGraphNodes()
{
	// Change Color of Nodes with no Connections
	// =========================================

	//std::vector<Elite::InfluenceNode> tempBlockedNode{ m_BlockedNodes };
	m_BlockedNodes.clear();

	const Elite::Color blockedNodeColor{ 1.f,1.f,1.f };

	// Go through all the Nodes
	for (auto& currentNode : m_pInfluenceGrid->GetAllNodes())
	{
		const auto nodeConnections{ m_pInfluenceGrid->GetNodeConnections(currentNode) };
		
		// If no Connections
		const bool hasNoConnections{ nodeConnections.size() == 0 };
		if (hasNoConnections)
		{
			// Set Color white and put in blockedNodes
			currentNode->SetColor(blockedNodeColor);
			m_BlockedNodes.push_back(currentNode);
		}
	}

	return m_BlockedNodes;
}

void InfluenceChasing::HandleInput(float deltaTime)
{
	//------ INPUT ------
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft))
	{
		AddInfluenceOnMouseClick(InputMouseButton::eLeft, 100);
	}
		
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eRight))
	{
		SetMouseTarget(InputMouseButton::eRight);
	}
}

void InfluenceChasing::HandleAgents(float deltaTime)
{
	// If lost Player and didn't start algorithm yet, do so
	const bool hasLostPlayer{ m_pAgentManager->GetHasLostPlayer() };
	if (hasLostPlayer && m_HasStartedAlgorithm == false)
	{
		m_HasStartedAlgorithm = true;

		// Get Player Info
		m_LastKnownPlayerPos = m_pAgentManager->GetPlayerPos();
		m_LastKnownPlayerVelocity = m_pAgentManager->GetPlayerVelocity();

		
	}
	// If Player was found, reset
	else if(hasLostPlayer == false)
	{
		m_HasStartedAlgorithm = false;
	}

	// Update Manager
	m_pAgentManager->Update(deltaTime);
}

void InfluenceChasing::AddInfluenceOnMouseClick(Elite::InputMouseButton mouseBtn, float inf)
{
	auto mouseData = INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, mouseBtn);
	auto mousePos = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld(Vector2{ (float)mouseData.X, (float)mouseData.Y });

	m_pInfluenceGrid->SetInfluenceAtPosition(mousePos, inf);
}
void InfluenceChasing::SetMouseTarget(Elite::InputMouseButton mouseBtn)
{
	auto mouseData = INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, mouseBtn);
	auto mousePos = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld(Vector2{ (float)mouseData.X, (float)mouseData.Y });

	m_pAgentManager->SetTarget(mousePos);
}
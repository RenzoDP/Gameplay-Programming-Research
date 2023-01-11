#include "stdafx.h"
#include "ChasingAlgorithm.h"

ChasingAlgorithm::ChasingAlgorithm(int cellAmount, int cellSize, bool isDirectionalGraph, bool isConnectedDiagonally)
{
	// Set InfluenceGrid
	m_pInfluenceGrid = new Elite::InfluenceMap<InfluenceGrid>(false);

	m_pInfluenceGrid->InitializeGrid(cellAmount, cellAmount, cellSize, isDirectionalGraph, isConnectedDiagonally);
	m_pInfluenceGrid->InitializeBuffer();

	m_GraphRenderer.SetNumberPrintPrecision(0);
}

ChasingAlgorithm::~ChasingAlgorithm()
{
	// Delete influenceGrid
	SAFE_DELETE(m_pInfluenceGrid);
}

void ChasingAlgorithm::Update(float deltaTime)
{
	// Check when should stop algorithm
	HandleAlgorithmStopping(deltaTime);

	// Propagate Barrier
	HandleBarrierPropagation(deltaTime);

	// Propagate Influence
	HandleInfluenceGrid(deltaTime);
}

void ChasingAlgorithm::Render(float deltaTime)
{
	// Set nodeColors
	m_pInfluenceGrid->SetNodeColorsBasedOnInfluence();

	// Render either Graph or Grid
	if (m_RenderAsGraph)
		m_GraphRenderer.RenderGraph(m_pInfluenceGrid, true, false, true, false);
	else
		m_GraphRenderer.RenderGraph(m_pInfluenceGrid, true, false, false, true);

	// Render Algorithm
	if (m_AlgorithmStarted)
	{
		DEBUGRENDERER2D->DrawPoint(m_StartPos, 10.f, { 1.f,0.f,1.f }, 0.09f);
		DEBUGRENDERER2D->DrawSegment(m_StartPos, m_StartPos + m_StartVelocity, { 1.f,1.f,1.f }, 0.091f);
	}
}

void ChasingAlgorithm::StartAlgorithm(const Elite::Vector2& startPos, const Elite::Vector2& startVelocity)
{
	m_AlgorithmStarted = true;

	// Set StartVariables
	// ------------
	m_StartPos = startPos;
	m_StartVelocity = startVelocity;

	// Set Barrier
	// ------------

	auto blockedNodes{ m_pInfluenceGrid->GetBlockedNodes() };

	const Elite::InfluenceNode* pStartNode{ m_pInfluenceGrid->GetNodeAtWorldPos(startPos) };

	// Loop through connections of currentNode
	for (const auto& currentNodeConnection : m_pInfluenceGrid->GetNodeConnections(pStartNode->GetIndex()))
	{
		Elite::InfluenceNode* pToNode{ m_pInfluenceGrid->GetNode(currentNodeConnection->GetTo()) };

		// Check if is not the Node you started from
		const bool isNotStartNode{ pStartNode != pToNode };
		if (isNotStartNode)
		{

			// Check if that node is behind the startPos
			const Elite::Vector2 toNodeDirection{ (m_pInfluenceGrid->GetNodeWorldPos(pToNode) - startPos).GetNormalized() };
			const float angleBetweenVelocityAndDirection{ Elite::AngleBetween(startVelocity,toNodeDirection) };

			const bool isBehindStartPos{ abs(angleBetweenVelocityAndDirection) >= m_MaxAngleInRadians };
			if (isBehindStartPos)
			{

				// Check if node isn't in blockedNodes
				bool hasFoundSameNode{ false };

				for (const auto& currentBlockedNode : blockedNodes)
				{
					const bool isSameNode{ currentBlockedNode == pToNode };
					if (isSameNode)
					{
						hasFoundSameNode = true;
					}
				}

				// If was already in blockedNodes, continue
				if (hasFoundSameNode) continue;


				//Else, put in blockedNodes and color
				pToNode->SetColor(m_BarrierNodeColor);
				blockedNodes.push_back(pToNode);

				m_AlgorithmBlockade.push_back(pToNode);
			}
		}
	}

	// Set new blockedNodes
	m_pInfluenceGrid->SetBlockedNodes(blockedNodes);
	
	// Set Influence
	// -------------
	m_pInfluenceGrid->SetInfluenceAtPosition(startPos, m_pInfluenceGrid->GetMaxHeat() * 20.f);
}
void ChasingAlgorithm::StopAlgorithm()
{
	m_AlgorithmStarted = false;

	// Clear Influence
	// ---------------
	for (auto& currentNode : m_pInfluenceGrid->GetAllNodes())
	{
		currentNode->SetInfluence(0.f);
	}

	// Reset Algorithm
	// ---------------
	m_pInfluenceGrid->SetShouldEndAlgorithm(false);

	// If there was no Blockade, return
	// --------------------------------
	const bool isBlockade(m_AlgorithmBlockade.size() != 0);
	if (isBlockade == false) return;

	// Reset Variables
	// ---------------
	m_StartPos = {};
	m_StartVelocity = {};

	// Delete algorithmBlockade Nodes out of blockedNodes
	// --------------------------------------------------
	auto oldBlockedNodes{ m_pInfluenceGrid->GetBlockedNodes() };
	std::vector<Elite::InfluenceNode*> newBlockedNodes{};

	// For every blocked
	for (const auto& currentOldBlockedNode : oldBlockedNodes)
	{
		// Check if Blocked was a Blockade
		bool wasBlockade{ false };
		for (const auto& currentAlgorithmBlockedNode : m_AlgorithmBlockade)
		{
			const bool isSameNode{ currentAlgorithmBlockedNode == currentOldBlockedNode };
			if (isSameNode)
			{
				wasBlockade = true;
			}
		}

		// If oldBlocked was an algorithmBlockade, don't add to the new blockade
		if (wasBlockade)
		{
			continue;
		}
		// Else, add to new blockade
		else
		{
			newBlockedNodes.push_back(currentOldBlockedNode);
		}
	}

	// Set new blockedNodes
	m_pInfluenceGrid->SetBlockedNodes(newBlockedNodes);

	// Clear Blockade
	m_AlgorithmBlockade.clear();
	m_BlockadeBuffer.clear();
}

Elite::InfluenceMap<InfluenceGrid>* ChasingAlgorithm::GetInfluenceGrid()
{
	return m_pInfluenceGrid;
}
void ChasingAlgorithm::SetBlockedNodes(const std::vector<Elite::InfluenceNode*>& blockedNodes)
{
	m_pInfluenceGrid->SetBlockedNodes(blockedNodes);
}

void ChasingAlgorithm::SetRenderAsGraph(bool renderAsGraph)
{
	m_RenderAsGraph = renderAsGraph;
}

float ChasingAlgorithm::GetMaxHeat() const
{
	return m_pInfluenceGrid->GetMaxHeat();
}
float ChasingAlgorithm::GetMaxAmountOfHeatedCells() const
{
	return m_pInfluenceGrid->GetMaxAmountOfHeatedCells();
}
float ChasingAlgorithm::GetMaxPropagationSteps() const
{
	return m_pInfluenceGrid->GetMaxPropagationSteps();
}
float ChasingAlgorithm::GetPropagationInterval() const
{
	return m_pInfluenceGrid->GetPropagationInterval();
}

void ChasingAlgorithm::SetMaxHeat(float maxHeat)
{
	m_pInfluenceGrid->SetMaxHeat(maxHeat);
}
void ChasingAlgorithm::SetMaxAmountOfHeatedCells(float maxAmountOfHeatedCells)
{
	m_pInfluenceGrid->SetMaxAmountOfHeatedCells(maxAmountOfHeatedCells);
}
void ChasingAlgorithm::SetMaxPropagationSteps(float maxPropagationSteps)
{
	m_pInfluenceGrid->SetMaxPropagationSteps(maxPropagationSteps);
}
void ChasingAlgorithm::SetPropagationInterval(float propagationInterval)
{
	m_pInfluenceGrid->SetPropagationInterval(propagationInterval);
}

void ChasingAlgorithm::SetInfluenceAtPosition(const Elite::Vector2 & position, float influence)
{
	m_pInfluenceGrid->SetInfluenceAtPosition(position, influence);
}

bool ChasingAlgorithm::GetStartedAlgorithm() const
{
	return m_AlgorithmStarted;
}
bool ChasingAlgorithm::GetIsStoppingAlgorithm() const
{
	return m_IsStoppingAlgorithm;
}

void ChasingAlgorithm::HandleAlgorithmStopping(float deltaTime)
{
	// Get variable from influenceGrid
	m_IsStoppingAlgorithm = m_pInfluenceGrid->GetShouldEndAlgorithm();

	// If shouldn't stop, return
	if (m_IsStoppingAlgorithm == false) return;

	// Else, reset after all Nodes have 0 influence
	bool noMoreInfluence{ true };
	for (auto& currentNode : m_pInfluenceGrid->GetAllNodes())
	{
		// If still has influence, break
		if (currentNode->GetInfluence() != 0)
		{
			noMoreInfluence = false;
			break;
		}
	}

	if (noMoreInfluence)
	{
		StopAlgorithm();
	}
}

void ChasingAlgorithm::HandleBarrierPropagation(float deltaTime)
{
	// If haven't started algorithm yet or should stop algorithm, return
	if (m_AlgorithmStarted == false || m_IsStoppingAlgorithm == true)
	{
		return;
	}

	const float propagationInterval{ m_pInfluenceGrid->GetPropagationInterval() };

	// After propagationInterval, Update
	// ----------------------------------
	m_TimeSinceLastPropagation += deltaTime;
	if (m_TimeSinceLastPropagation > propagationInterval)
	{
		m_TimeSinceLastPropagation -= propagationInterval;

		// Clear Buffer
		// ------------
		m_BlockadeBuffer.clear();

		// Get mapBlockades
		// ----------------
		auto blockedNodes{ m_pInfluenceGrid->GetBlockedNodes() };

		// Loop over the Nodes
		for (auto& currentNode : m_pInfluenceGrid->GetAllNodes())
		{
			// Check if is playerStartNode
			// ---------------------------

			const auto playerStartNode{ m_pInfluenceGrid->GetNodeAtWorldPos(m_StartPos) };
			const bool isPlayerStartNode{ currentNode == playerStartNode };
			if (isPlayerStartNode) continue;

			// Check if is blockade
			// --------------------
			bool isBlockadeNode{ false };

			// Loop through blockade
			for (const auto& currentBlockadeNode : blockedNodes)
			{
				const bool isSameNode{ currentNode == currentBlockadeNode };
				if (isSameNode)
				{
					isBlockadeNode = true;
					break;
				}
			}

			// If is blockadeNode, continue
			if (isBlockadeNode) continue;


			// Check if has blockadeNeighbors
			// ------------------------------
			bool hasBlockadeNeighbor{ false };

			// Loop through nodeConnections
			const auto nodeConnections{ m_pInfluenceGrid->GetNodeConnections(currentNode) };

			for (const auto& currentNodeConnection : nodeConnections)
			{
				const Elite::InfluenceNode* pNextNode{ m_pInfluenceGrid->GetNode(currentNodeConnection->GetTo()) };
				
				// Loop through blockade
				for (const auto& currentBlockadeNode : m_AlgorithmBlockade)
				{
					// If is same node, currentNode has a blockadeNeighbor
					const bool isSameNode{ pNextNode->GetIndex() == currentBlockadeNode->GetIndex()};
					if (isSameNode)
					{
						hasBlockadeNeighbor = true;
						break;
					}
				}

				if (hasBlockadeNeighbor) break;
			}

			// If doesn't have blockadeNeighbor, continue
			if (hasBlockadeNeighbor == false) continue;


			// Check if is behind Player and isn't the playerStartNode
			// -------------------------------------------------------
			const Elite::Vector2 toNodeDirection{ (m_pInfluenceGrid->GetNodeWorldPos(currentNode) - m_StartPos).GetNormalized() };
			const float angleBetweenVelocityAndDirection{ Elite::AngleBetween(m_StartVelocity,toNodeDirection) };

			// If is behind startPos, add to Buffer
			const bool isBehindStartPos{ abs(angleBetweenVelocityAndDirection) >= m_MaxAngleInRadians };
			if (isBehindStartPos)
			{
				m_BlockadeBuffer.push_back(currentNode);
			}
		}

		// Set new blockade
		// ----------------

		// Loop through Buffer
		for (auto& currentBufferNode : m_BlockadeBuffer)
		{
			// Push_back for new blockade and algorithmBlockade
			blockedNodes.push_back(currentBufferNode);
			m_AlgorithmBlockade.push_back(currentBufferNode);

			// Set new Color
			currentBufferNode->SetColor(m_BarrierNodeColor);

			// Remove Connections
			m_pInfluenceGrid->RemoveConnectionsToAdjacentNodes(currentBufferNode->GetIndex());
		}

		// Set new blockade
		m_pInfluenceGrid->SetBlockedNodes(blockedNodes);
	}
}

void ChasingAlgorithm::HandleInfluenceGrid(float deltaTime)
{
	if (m_AlgorithmStarted == false)
	{
		return;
	}

	// Propagate Influence
	m_pInfluenceGrid->PropagateInfluence(deltaTime);
}
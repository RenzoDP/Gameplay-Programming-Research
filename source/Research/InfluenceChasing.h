#pragma once
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"

//--- Includes and Forward Declarations ---
#include "projects/Movement/SteeringBehaviors/SteeringHelpers.h"
#include "framework\EliteAI\EliteGraphs\EInfluenceMap.h"
#include "framework\EliteAI\EliteGraphs\EGraph2D.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphEditor.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"

#include "projects/Movement/SteeringBehaviors/SteeringAgent.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

using InfluenceGrid = Elite::GridGraph<Elite::InfluenceNode, Elite::GraphConnection>;
class AgentManager;
class ChasingAlgorithm;

class InfluenceChasing final
{
public:
	// Constructor and Destructor
	explicit InfluenceChasing(Elite::InfluenceMap<InfluenceGrid>* pInfluenceGrid, const std::vector<Elite::Vector2>& worldMaxima,
							  int cellAmount, int cellSize, bool isDirectionalGraph, bool isConnectedDiagonally);
	~InfluenceChasing();

	// Rule of Five
	InfluenceChasing(const InfluenceChasing&) = delete;
	InfluenceChasing& operator=(const InfluenceChasing&) = delete;
	InfluenceChasing(InfluenceChasing&&) = delete;
	InfluenceChasing& operator=(InfluenceChasing&&) = delete;

	// Public Functions
	void Update(float deltaTime);
	void Render(float deltaTime);

	std::vector<Elite::InfluenceNode*> UpdateGraphNodes(Elite::GraphEditor& m_GridEditor);

	// Getters/Setters
	void SetRenderAsGraph(bool renderAsGraph);

	float GetMaxHeat() const;
	float GetMaxAmountOfHeatedCells() const;
	float GetMaxPropagationSteps() const;
	float GetPropagationInterval() const;

	void SetMaxHeat(float maxHeat);
	void SetMaxAmountOfHeatedCells(float maxAmountOfHeatedCells);
	void SetMaxPropagationSteps(float maxPropagationSteps);
	void SetPropagationInterval(float propagationInterval);

	void SetRenderAgents(bool renderAgents);

private:

	// Grid-Variables
	Elite::InfluenceMap<InfluenceGrid>* m_pInfluenceGrid{ nullptr };
	const Elite::Color m_BaseNodeColor{ 0.7f,0.7f,0.7f };

	// Agent Manager
	bool m_RenderAgents{ true };

	AgentManager* m_pAgentManager{ nullptr };
	bool m_HasLostPlayer{ true };

	// Chasing Algorithm
	ChasingAlgorithm* m_pChasingAlgorithm{ nullptr };

	// Debug
	Elite::Vector2 m_MouseClickPlayerPos{};
	Elite::Vector2 m_MouseClickVelocityPos{};
	int m_TimesClicked{};

	// Helper
	void HandleInput(float deltaTime);
	void HandleAgents(float deltaTime);

	void SetMouseTarget(Elite::InputMouseButton mouseBtn);
	Elite::Vector2 GetMouseTarget(Elite::InputMouseButton mouseBtn) const;

	void AddBlockedNodes(Elite::InfluenceMap<InfluenceGrid>* pInfluenceGrid, std::vector<Elite::InfluenceNode*>& blockedNodes, bool storeInBlockedNodes);
};


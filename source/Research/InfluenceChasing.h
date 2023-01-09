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

class InfluenceChasing final
{
public:
	// Constructor and Destructor
	explicit InfluenceChasing(Elite::InfluenceMap<InfluenceGrid>* pInfluenceGrid, const std::vector<Elite::Vector2>& worldMaxima);
	~InfluenceChasing();

	// Rule of Five
	InfluenceChasing(const InfluenceChasing&) = delete;
	InfluenceChasing& operator=(const InfluenceChasing&) = delete;
	InfluenceChasing(InfluenceChasing&&) = delete;
	InfluenceChasing& operator=(InfluenceChasing&&) = delete;

	// Public Functions
	void Update(float deltaTime);
	void Render(float deltaTime);

	std::vector<Elite::InfluenceNode*> UpdateGraphNodes();

private:

	// Grid-Variables
	Elite::InfluenceMap<InfluenceGrid>* m_pInfluenceGrid{ nullptr };
	const Elite::Color m_BaseNodeColor{ 0.7f,0.7f,0.7f };

	std::vector<Elite::InfluenceNode*> m_BlockedNodes{};

	// Agent Manager
	AgentManager* m_pAgentManager{ nullptr };
	bool m_HasStartedAlgorithm{ false };

	Elite::Vector2 m_LastKnownPlayerPos{};
	Elite::Vector2 m_LastKnownPlayerVelocity{};

	// Helper
	void HandleInput(float deltaTime);
	void HandleAgents(float deltaTime);

	void AddInfluenceOnMouseClick(Elite::InputMouseButton mouseBtn, float inf);
	void SetMouseTarget(Elite::InputMouseButton mouseBtn);
};


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

using InfluenceGrid = Elite::GridGraph<Elite::InfluenceNode, Elite::GraphConnection>;

class ChasingAlgorithm final
{
public:
	// Constructor and Destructor
	explicit ChasingAlgorithm(int cellAmount, int cellSize, bool isDirectionalGraph, bool isConnectedDiagonally);
	~ChasingAlgorithm();

	// Rule of Five
	ChasingAlgorithm(const ChasingAlgorithm&) = delete;
	ChasingAlgorithm& operator=(const ChasingAlgorithm&) = delete;
	ChasingAlgorithm(ChasingAlgorithm&&) = delete;
	ChasingAlgorithm& operator=(ChasingAlgorithm&&) = delete;

	// Public functions
	void Update(float deltaTime);
	void Render(float deltaTime);

	void StartAlgorithm(const Elite::Vector2& startPos, const Elite::Vector2& startVelocity);
	void StopAlgorithm();

	// Getters/Setters
	Elite::InfluenceMap<InfluenceGrid>* GetInfluenceGrid();
	void SetBlockedNodes(const std::vector<Elite::InfluenceNode*>& blockedNodes);

	void SetRenderAsGraph(bool renderAsGraph);

	float GetMaxHeat() const;
	float GetMaxAmountOfHeatedCells() const;
	float GetMaxPropagationSteps() const;
	float GetPropagationInterval() const;

	void SetMaxHeat(float maxHeat);
	void SetMaxAmountOfHeatedCells(float maxAmountOfHeatedCells);
	void SetMaxPropagationSteps(float maxPropagationSteps);
	void SetPropagationInterval(float propagationInterval);

	void SetInfluenceAtPosition(const Elite::Vector2& position, float influence);

	bool GetStartedAlgorithm() const;
	bool GetIsStoppingAlgorithm() const;

private:
	// Variables
	// ---------

	// Algorithm Started
	bool m_AlgorithmStarted{ false };
	bool m_IsStoppingAlgorithm{ false };

	// Copy of InfluenceGrid
	Elite::InfluenceMap<InfluenceGrid>* m_pInfluenceGrid{ nullptr };
	Elite::GraphRenderer m_GraphRenderer{};

	// Algorithm blockade
	std::vector<Elite::InfluenceNode*> m_AlgorithmBlockade{};
	
	float m_TimeSinceLastPropagation{};
	std::vector<Elite::InfluenceNode*> m_BlockadeBuffer{};

	const Elite::Color m_BarrierNodeColor{ 1.f,0.8f,0.f };
	const float m_MaxAngleInRadians{ Elite::ToRadians(75.f) };

	// Algorithm startPos
	Elite::Vector2 m_StartPos{};
	Elite::Vector2 m_StartVelocity{};

	// Rendering
	bool m_RenderAsGraph{ false };

	// Functions
	// ---------

	void HandleAlgorithmStopping(float deltaTime);
	void HandleBarrierPropagation(float deltaTime);
	void HandleInfluenceGrid(float deltaTime);
};


#pragma once
//--- Includes and Forward Declarations ---
#include "projects/Movement/SteeringBehaviors/SteeringHelpers.h"
#include "framework\EliteAI\EliteGraphs\EInfluenceMap.h"
#include "framework\EliteAI\EliteGraphs\EGraph2D.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphEditor.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"

#include "projects/Movement/SteeringBehaviors/SteeringAgent.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

namespace Elite
{
	class NavGraph;
}
using InfluenceGrid = Elite::GridGraph<Elite::InfluenceNode, Elite::GraphConnection>;

class AgentManager final
{
public:
	// Enum
	enum GuardBehavior
	{
		OnGuard, Chasing, Alert
	};

	// Constructor and Destructor
	explicit AgentManager(Elite::InfluenceMap<InfluenceGrid>* pInfluenceGrid, const std::vector<Elite::Vector2>& worldMaxima);
	~AgentManager();

	// Rule of Five
	AgentManager(const AgentManager&) = delete;
	AgentManager& operator=(const AgentManager&) = delete;
	AgentManager(AgentManager&&) = delete;
	AgentManager& operator=(AgentManager&&) = delete;
	
	// Public Functions
	void Update(float deltaTime, bool algorithmIsBusy);
	void Render(float deltaTime);

	// Getters/Setters
	void SetPlayerTarget(const Elite::Vector2& newTarget);
	void SetGuardTarget(const Elite::Vector2& newTarget);

	bool GetHasLostPlayer() const;
	Elite::Vector2 GetPlayerPos() const;
	Elite::Vector2 GetPlayerVelocity() const;

	GuardBehavior GetCurrentGuardBehavior() const;

private:

	// Variables
	// =========

	// InfluenceGrid
	// -------------
	Elite::InfluenceMap<InfluenceGrid>* m_pInfluenceGrid{ nullptr };


	// Agents
	// --------
	StandStill* m_pStandStillBehavior{ nullptr };

	// Player Agent
	SteeringAgent* m_pPlayerAgent{ nullptr };
	const float m_MaxPlayerLinearSpeed{ 15.f };
	Elite::Vector2 m_PlayerTarget{};

	Seek* m_pPlayerSeekingBehavior{ nullptr };
	Arrive* m_pPlayerArriveBehavior{ nullptr };

	std::vector<Elite::InfluenceNode*> m_vPlayerPath{};

	// Guard Agent
	SteeringAgent* m_pGuardAgent{ nullptr };
	const float m_MaxGuardLinearSpeed{ 7.5f };
	Elite::Vector2 m_GuardTarget{};

	GuardBehavior m_CurrentGuardBehavior{ OnGuard };
	Seek* m_pGuardSeekingBehavior{ nullptr };

	std::vector<Elite::InfluenceNode*> m_vGuardPath{};

	// Cone
	Elite::Vector2 m_LeftConePoint{};
	Elite::Vector2 m_RightConePoint{};

	float m_LeftConeAngleDifference{};
	float m_RightConeAngleDifference{};

	const float m_ConeLength{25.f};
	const float m_FOV{ 45.f };

	// Guard Colors
	const Elite::Color m_GuardNeutralColor{ 0,1.f,0 };
	const Elite::Color m_GuardAlertColor{ 1.f,1.f,0 };
	const Elite::Color m_GuardChasingColor{ 1.f,0,0 };


	// Vision
	// ------

	bool m_HasSeenPlayerAlready{ false };
	bool m_HasLostPlayer{ false };

	// Functions
	// =========

	void InitializeAgents(const std::vector<Elite::Vector2>& worldMaxima);

	void UpdatePlayer(float deltaTime);
	void UpdateGuard(float deltaTime, bool algorithmIsBusy);

	void CalculatePath(const Elite::Vector2& startPos, const Elite::Vector2& endPos, std::vector<Elite::InfluenceNode*>& path, bool isPlayer);
	void FollowPath(SteeringAgent* pAgent, ISteeringBehavior* pSeekingBehavior, ISteeringBehavior* pArriveBehavior, std::vector<Elite::InfluenceNode*>& path);
};


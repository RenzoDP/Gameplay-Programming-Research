#pragma once
#include "projects/Movement/SteeringBehaviors/SteeringAgent.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

namespace Elite
{
	class NavGraph;
}

class AgentManager final
{
public:
	// Constructor and Destructor
	explicit AgentManager(const std::vector<Elite::Vector2>& worldMaxima);
	~AgentManager();

	// Rule of Five
	AgentManager(const AgentManager&) = delete;
	AgentManager& operator=(const AgentManager&) = delete;
	AgentManager(AgentManager&&) = delete;
	AgentManager& operator=(AgentManager&&) = delete;
	
	// Public Functions
	void Update(float deltaTime);
	void Render(float deltaTime);

	// Getters/Setters
	void SetPlayerTarget(const Elite::Vector2& newTarget);
	void SetGuardTarget(const Elite::Vector2& newTarget);

	bool GetHasLostPlayer() const;
	Elite::Vector2 GetPlayerPos() const;
	Elite::Vector2 GetPlayerVelocity() const;


private:

	// Variables
	// =========

	// Agents
	// --------

	// Player Agent
	SteeringAgent* m_pPlayerAgent{ nullptr };
	const float m_MaxPlayerLinearSpeed{ 15.f };
	Elite::Vector2 m_PlayerTarget{};

	Seek* m_pPlayerSeekingBehavior{ nullptr };
	Arrive* m_pPlayerArriveBehavior{ nullptr };

	// Guard Agent
	SteeringAgent* m_pGuardAgent{ nullptr };
	const float m_MaxGuardLinearSpeed{ 7.5f };
	Elite::Vector2 m_GuardTarget{};

	Seek* m_pGuardSeekingBehavior{ nullptr };
	StandStill* m_pGuardStandStill{ nullptr };

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

	void InitializeAgents();

	void UpdatePlayer(float deltaTime);
	void UpdateGuard(float deltaTime);
};


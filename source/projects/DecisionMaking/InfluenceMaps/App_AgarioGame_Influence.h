#ifndef AGARIO_GAME_APPLICATION_H
#define AGARIO_GAME_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "framework\EliteAI\EliteGraphs\EInfluenceMap.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"

class AgarioFood;
class AgarioAgent;
class AgarioContactListener;
class NavigationColliderElement;

using InfluenceGrid = Elite::GridGraph<Elite::InfluenceNode, Elite::GraphConnection>;

class App_AgarioGame_Influence final : public IApp
{
public:
	App_AgarioGame_Influence();
	~App_AgarioGame_Influence();

	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;
private:
	float m_TrimWorldSize = 150.f;
	const int m_AmountOfAgents{ 20 };
	std::vector<AgarioAgent*> m_pAgentVec{};

	AgarioAgent* m_pSmartAgent = nullptr;

	const int m_AmountOfFood{ 40 };
	const float m_FoodSpawnDelay{ 2.f };
	float m_TimeSinceLastFoodSpawn{ 0.f };
	std::vector<AgarioFood*> m_pFoodVec{};

	AgarioContactListener* m_pContactListener = nullptr;
	bool m_GameOver = false;

	// -- Influence Map -- //
	Elite::GraphRenderer m_GraphRenderer{};
	Elite::InfluenceMap<InfluenceGrid>* m_pInfluenceGrid;

	bool m_ShowInfluenceMap{ false };

	//--Level--
	std::vector<NavigationColliderElement*> m_vNavigationColliders = {};
private:	
	template<class T_AgarioType>
	void UpdateAgarioEntities(std::vector<T_AgarioType*>& entities, float deltaTime);

	Elite::Blackboard* CreateBlackboard(AgarioAgent* a);
	void UpdateImGui();

	void UpdateInfluenceMap();
private:
	//C++ make the class non-copyable
	App_AgarioGame_Influence(const App_AgarioGame_Influence&) {};
	App_AgarioGame_Influence& operator=(const App_AgarioGame_Influence&) {};
};

template<class T_AgarioType>
inline void App_AgarioGame_Influence::UpdateAgarioEntities(std::vector<T_AgarioType*>& entities, float deltaTime)
{
	for (auto& e : entities)
	{
		e->Update(deltaTime);

		if (e->CanBeDestroyed())
			SAFE_DELETE(e);
	}

	auto toRemoveEntityIt = std::remove_if(entities.begin(), entities.end(),
		[](T_AgarioType* e) {return e == nullptr; });
	if (toRemoveEntityIt != entities.end())
	{
		entities.erase(toRemoveEntityIt, entities.end());
	}
}
#endif
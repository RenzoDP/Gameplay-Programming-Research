#include "stdafx.h"
#include "App_AgarioGame.h"
#include "StatesAndTransitions.h"


//AgarioIncludes
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioContactListener.h"


using namespace Elite;
using namespace FSMStates;
using namespace FSMConditions;

App_AgarioGame::App_AgarioGame()
{
}

App_AgarioGame::~App_AgarioGame()
{
	for (auto& f : m_pFoodVec)
	{
		SAFE_DELETE(f);
	}
	m_pFoodVec.clear();

	for (auto& a : m_pAgentVec)
	{
		SAFE_DELETE(a);
	}
	m_pAgentVec.clear();

	SAFE_DELETE(m_pContactListener);
	SAFE_DELETE(m_pSmartAgent);
	for (auto& s : m_pStates)
	{
		SAFE_DELETE(s);
	}

	for (auto& t : m_pConditions)
	{
		SAFE_DELETE(t);
	}

}

void App_AgarioGame::Start()
{
	//Creating the world contact listener that informs us of collisions
	m_pContactListener = new AgarioContactListener();

	//Create food items
	m_pFoodVec.reserve(m_AmountOfFood);
	for (int i = 0; i < m_AmountOfFood; i++)
	{
		Elite::Vector2 randomPos = randomVector2(0, m_TrimWorldSize);
		m_pFoodVec.push_back(new AgarioFood(randomPos));
	}

	// Wander state for every agent
	WanderState* pWanderState = new WanderState();
	m_pStates.push_back(pWanderState);

	//Create default agents
	m_pAgentVec.reserve(m_AmountOfAgents);
	for (int i = 0; i < m_AmountOfAgents; i++)
	{
		Elite::Vector2 randomPos = randomVector2(0, m_TrimWorldSize * (2.0f / 3));
		AgarioAgent* pNewAgent = new AgarioAgent(randomPos);

		Blackboard* pBlackboard = CreateBlackboard(pNewAgent);

		FiniteStateMachine* pStateMachine = new FiniteStateMachine(pWanderState, pBlackboard);
		pNewAgent->SetDecisionMaking(pStateMachine);

		m_pAgentVec.push_back(pNewAgent);
	}

	

	//-------------------
	//Create Custom Agent
	//-------------------
	Elite::Vector2 randomPos = randomVector2(0, m_TrimWorldSize * (2.0f / 3));
	Color customColor = Color{ 0.0f, 1.0f, 0.0f };
	m_pSmartAgent = new AgarioAgent(randomPos, customColor);

	//1. Create and add the necessary blackboard data
	Blackboard* pBlackboard = CreateBlackboard(m_pSmartAgent);

	//2. Create the different agent states
	SeekFoodState* pSeekFoodState = new SeekFoodState();
	m_pStates.push_back(pSeekFoodState);

	EvadeBorder* pEvadeBorder = new EvadeBorder();
	m_pStates.push_back(pEvadeBorder);
	
	EvadeBiggerAgent* pEvadeBiggerAgent = new EvadeBiggerAgent();
	m_pStates.push_back(pEvadeBiggerAgent);

	ChaseSmallerAgent* pChaseSmallerAgent = new ChaseSmallerAgent();
	m_pStates.push_back(pChaseSmallerAgent);

	//3. Create the conditions beetween those states
	FoodNearbyCondition* pFoodNearbyCondition = new FoodNearbyCondition();
	m_pConditions.push_back(pFoodNearbyCondition);

	FoundFoodEatenCondition* pFoundFoodEatenCondition = new FoundFoodEatenCondition();
	m_pConditions.push_back(pFoundFoodEatenCondition);

	BorderNearbyCondition* pBorderNearbyCondition = new BorderNearbyCondition();
	m_pConditions.push_back(pBorderNearbyCondition);

	FarEnoughFromBorderCondition* pFarEnoughFromBorderCondition = new FarEnoughFromBorderCondition();
	m_pConditions.push_back(pFarEnoughFromBorderCondition);

	BiggerAgentAroundCondition* pBiggerAgentAroundCondition = new BiggerAgentAroundCondition();
	m_pConditions.push_back(pBiggerAgentAroundCondition);

	BiggerAgentFarEnoughCondition* pBiggerAgentFarEnoughCondition = new BiggerAgentFarEnoughCondition();
	m_pConditions.push_back(pBiggerAgentFarEnoughCondition);

	SmallerAgentAroundCondition* pSmallerAgentAroundCondition = new SmallerAgentAroundCondition();
	m_pConditions.push_back(pSmallerAgentAroundCondition);

	SmallerAgentEaten* pSmallerAgentEaten = new SmallerAgentEaten();
	m_pConditions.push_back(pSmallerAgentEaten);

	//4. Create the finite state machine with a starting state and the blackboard
	FiniteStateMachine* pStateMachine = new FiniteStateMachine(pWanderState, pBlackboard);

	//5. Add the transitions for the states to the state machine
	// stateMachine->AddTransition(startState, toState, condition)
	// startState: active state for which the transition will be checked
	// condition: if the Evaluate function returns true => transition will fire and move to the toState
	// toState: end state where the agent will move to if the transition fires
	
	pStateMachine->AddTransition(pWanderState, pSeekFoodState, pFoodNearbyCondition);
	pStateMachine->AddTransition(pWanderState, pEvadeBorder, pBorderNearbyCondition);
	pStateMachine->AddTransition(pWanderState, pEvadeBiggerAgent, pBiggerAgentAroundCondition);
	pStateMachine->AddTransition(pWanderState, pChaseSmallerAgent, pSmallerAgentAroundCondition);

	pStateMachine->AddTransition(pSeekFoodState, pWanderState, pFoundFoodEatenCondition);
	pStateMachine->AddTransition(pSeekFoodState, pEvadeBiggerAgent, pBiggerAgentAroundCondition);

	pStateMachine->AddTransition(pEvadeBorder, pWanderState, pFarEnoughFromBorderCondition);

	pStateMachine->AddTransition(pEvadeBiggerAgent, pWanderState, pBiggerAgentFarEnoughCondition);

	pStateMachine->AddTransition(pChaseSmallerAgent, pWanderState, pSmallerAgentEaten);

	//6. Activate the decision making stucture on the custom agent by calling the SetDecisionMaking function
	m_pSmartAgent->SetDecisionMaking(pStateMachine);
}

void App_AgarioGame::Update(float deltaTime)
{
	UpdateImGui();

	//Check if agent is still alive
	if (m_pSmartAgent->CanBeDestroyed())
	{
		m_GameOver = true;

		//Update the other agents and food
		UpdateAgarioEntities(m_pFoodVec, deltaTime);
		UpdateAgarioEntities(m_pAgentVec, deltaTime);
		return;
	}

	//Update the custom agent
	m_pSmartAgent->Update(deltaTime);
	m_pSmartAgent->TrimToWorld(m_TrimWorldSize, false);

	//Update the other agents and food
	UpdateAgarioEntities(m_pFoodVec, deltaTime);
	UpdateAgarioEntities(m_pAgentVec, deltaTime);
	
	//Check if we need to spawn new food
	m_TimeSinceLastFoodSpawn += deltaTime;
	if (m_TimeSinceLastFoodSpawn > m_FoodSpawnDelay)
	{
		m_TimeSinceLastFoodSpawn = 0.f;
		m_pFoodVec.push_back(new AgarioFood(randomVector2(0, m_TrimWorldSize)));
	}
}

void App_AgarioGame::Render(float deltaTime) const
{
	RenderWorldBounds(m_TrimWorldSize);

	for (AgarioFood* f : m_pFoodVec)
	{
		f->Render(deltaTime);
	}

	for (AgarioAgent* a : m_pAgentVec)
	{
		a->Render(deltaTime);
	}

	m_pSmartAgent->Render(deltaTime);
}

Blackboard* App_AgarioGame::CreateBlackboard(AgarioAgent* a)
{
	Blackboard* pBlackboard = new Blackboard();
	pBlackboard->AddData("Agent", a);
	pBlackboard->AddData("FoodVec", &m_pFoodVec);
	pBlackboard->AddData("FoodNearby", static_cast<AgarioFood*>(nullptr));
	pBlackboard->AddData("FoodEaten", false);
	pBlackboard->AddData("TrimWorldSize", m_TrimWorldSize);
	pBlackboard->AddData("AgentsVec", &m_pAgentVec);
	pBlackboard->AddData("AgentToEvade", static_cast<AgarioAgent*>(nullptr));
	pBlackboard->AddData("AgentToChase", static_cast<AgarioAgent*>(nullptr));

	return pBlackboard;
}

void App_AgarioGame::UpdateImGui()
{
	//------- UI --------
#ifdef PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth = 150;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 90));
		ImGui::Begin("Agario", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();
		
		ImGui::Text("Agent Info");
		ImGui::Text("Radius: %.1f",m_pSmartAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());
		
		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
	if(m_GameOver)
	{
		//Setup
		int menuWidth = 300;
		int menuHeight = 100;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2(width/2.0f- menuWidth, height/2.0f - menuHeight));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)menuHeight));
		ImGui::Begin("Game Over", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		ImGui::Text("Final Agent Info");
		ImGui::Text("Radius: %.1f", m_pSmartAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());
		ImGui::End();
	}
#pragma endregion
#endif

}

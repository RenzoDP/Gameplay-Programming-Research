//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"
using namespace Elite;

//Includes
#include "App_InfluenceMap.h"
#include "projects/Movement/SteeringBehaviors/SteeringAgent.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "Research/InfluenceChasing.h"

//Destructor
App_InfluenceMap::~App_InfluenceMap()
{
	SAFE_DELETE(m_pInfluenceGrid);

	SAFE_DELETE(m_pInfluenceChasing);
}

//Functions
void App_InfluenceMap::Start()
{
	const int cellSize{ 5 };
	const int cellAmount{ 25 };

	std::vector<Elite::Vector2> worldMaxima{};
	worldMaxima.push_back(Elite::Vector2{ 0.f,0.f });
	worldMaxima.push_back(Elite::Vector2{ static_cast<float>(cellSize) * cellAmount,static_cast<float>(cellSize) * cellAmount });

	//Initialization of your application. If you want access to the physics world you will need to store it yourself.
	//----------- CAMERA ------------
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(50.f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(worldMaxima[1] / 2.f);
	DEBUGRENDERER2D->GetActiveCamera()->SetMoveLocked(false);

	m_pInfluenceGrid = new InfluenceMap<InfluenceGrid>(false);
	m_pInfluenceGrid->InitializeGrid(cellAmount, cellAmount, cellSize, false, true);
	m_pInfluenceGrid->InitializeBuffer();

	m_GraphRenderer.SetNumberPrintPrecision(0);

	// Own Stuff
	// =========

	m_pInfluenceChasing = new InfluenceChasing(m_pInfluenceGrid, worldMaxima, cellAmount, cellSize, false, true);
}

void App_InfluenceMap::Update(float deltaTime)
{
	if (m_EditGraphEnabled)
	{
		// If Graph has changed
		if (m_GridEditor.UpdateGraph(m_pInfluenceGrid))
		{
			// Don't change Color from changed Node
			auto blockedNodes{ m_pInfluenceChasing->UpdateGraphNodes(m_GridEditor) };
			m_pInfluenceGrid->SetBlockedNodes(blockedNodes);
		}
	}
	else
	{
		m_pInfluenceChasing->SetRenderAgents(m_RenderAgents);
		m_pInfluenceChasing->Update(deltaTime);
	}

	UpdateUI();
}

void App_InfluenceMap::UpdateUI()
{
	//Setup
	int menuWidth = 200;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 90));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
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

	ImGui::Text("Influence Maps");
	ImGui::Spacing();
	ImGui::Spacing();

	
	ImGui::Checkbox("Enable graph editing", &m_EditGraphEnabled);
	ImGui::Checkbox("Render as graph", &m_RenderAsGraph);
	ImGui::Checkbox("Render agents", &m_RenderAgents);

	//Get influence map data
	auto maxHeat = m_pInfluenceChasing->GetMaxHeat();
	auto maxAmountOfHeatedCells = m_pInfluenceChasing->GetMaxAmountOfHeatedCells();
	auto maxPropagationSteps = m_pInfluenceChasing->GetMaxPropagationSteps();
	auto propagationInterval = m_pInfluenceChasing->GetPropagationInterval();

	auto startHeat{ maxHeat };

	ImGui::SliderFloat("Max Heat", &maxHeat, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Max amount of heated Cells", &maxAmountOfHeatedCells, 0.0f, 1.f, "%.2");
	ImGui::SliderFloat("Max Propagation steps", &maxPropagationSteps, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Propagation Interval", &propagationInterval, 0.f, 2.f, "%.2");
	ImGui::Spacing();

	//Set data
	m_pInfluenceChasing->SetMaxHeat(maxHeat);
	m_pInfluenceChasing->SetMaxAmountOfHeatedCells(maxAmountOfHeatedCells);
	m_pInfluenceChasing->SetMaxPropagationSteps(maxPropagationSteps);
	m_pInfluenceChasing->SetPropagationInterval(propagationInterval);

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
}

void App_InfluenceMap::Render(float deltaTime) const
{
	m_pInfluenceChasing->SetRenderAsGraph(m_RenderAsGraph);

	m_pInfluenceChasing->Render(deltaTime);
}

#include "stdafx.h"
#include "Flock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"
#include "projects/Movement/SteeringBehaviors/SpacePartitioning/SpacePartitioning.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 5 }
	, m_NrOfNeighbors{0}
{
	// Init Vectors
	m_Agents.resize(m_FlockSize);
	m_Neighbors.resize(m_FlockSize);
	m_OldAgentPos.resize(m_FlockSize);

	// Init Behaviors
	m_pSeekBehavior = new Seek();
	m_pWanderBehavior = new Wander();
	m_pCohesionBehavior = new Cohesion(this);
	m_pSeparationBehavior = new Separation(this);
	m_pVelMatchBehavior = new VelocityMatch(this);
	m_pEvadeBehavior = new Evade();

	m_pBlendedSteering = new BlendedSteering({ {m_pSeparationBehavior,0.5f}
												,{m_pVelMatchBehavior,0.5f}
												,{m_pCohesionBehavior,0.5f}
												, {m_pWanderBehavior, 0.5f} 
												, {m_pSeekBehavior, 0.5f} });
	m_pPrioritySteering = new PrioritySteering({ m_pEvadeBehavior,m_pBlendedSteering });

	// Init Flock
	Vector2 randomPos{};
	for (int idx{}; idx < m_FlockSize; idx++)
	{
		randomPos.x = (rand() % int(m_WorldSize * 100)) / 100.f;
		randomPos.y = (rand() % int(m_WorldSize * 100)) / 100.f;

		m_Agents[idx] = new SteeringAgent();

		m_Agents[idx]->SetPosition(randomPos);
		m_Agents[idx]->SetSteeringBehavior(m_pPrioritySteering);
		m_Agents[idx]->SetMaxLinearSpeed(55.f);
		m_Agents[idx]->SetMaxAngularSpeed(25.f);
		m_Agents[idx]->SetAutoOrient(true);
		m_Agents[idx]->SetMass(1.f);
		//m_Agents[idx]->SetBodyColor({ 1,0,1 });
	}

	// Init Agent to evade
	m_pAgentToEvade = new SteeringAgent();

	m_pAgentToEvade->SetSteeringBehavior(m_pSeekBehavior);
	m_pAgentToEvade->SetMaxLinearSpeed(15.f);
	m_pAgentToEvade->SetAutoOrient(true);
	m_pAgentToEvade->SetBodyColor({ 1,1,0 });

	// Init CellSpace
	const int nrRows{ 25 };
	const int nrCols{ nrRows };

	m_pCellSpace = new CellSpace(m_WorldSize, m_WorldSize, nrRows, nrCols, m_FlockSize);
	for (int idx{}; idx < m_FlockSize; idx++)
	{
		m_pCellSpace->AddAgent(m_Agents[idx]);
	}
}

Flock::~Flock()
{
	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);

	for(auto pAgent: m_Agents)
	{
		SAFE_DELETE(pAgent);
	}
	m_Agents.clear();

	for (size_t idx{}; idx < m_Neighbors.size(); idx++)
	{
		m_Neighbors[idx] = nullptr;
	}
	m_Neighbors.clear();

	SAFE_DELETE(m_pAgentToEvade);

	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pWanderBehavior);
	SAFE_DELETE(m_pCohesionBehavior);
	SAFE_DELETE(m_pSeparationBehavior);
	SAFE_DELETE(m_pVelMatchBehavior);
	SAFE_DELETE(m_pEvadeBehavior);

	SAFE_DELETE(m_pCellSpace);
}

void Flock::Update(float deltaT)
{	
	// Entire flock
	for (int idx{}; idx < m_FlockSize; idx++)
	{
		// Update CellSpace
		m_pCellSpace->UpdateAgentCell(m_Agents[idx], m_OldAgentPos[idx]);

		// Store positions in oldVector
		m_OldAgentPos[idx] = m_Agents[idx]->GetPosition();

		// Use either normal registerNeighbors or spacePartitioning one
		if (m_UseSpacePartitioning)
		{
			m_pCellSpace->RegisterNeighbors(m_Agents[idx], m_NeighborhoodRadius);
			
			m_NrOfNeighbors = m_pCellSpace->GetNrOfNeighbors();
			m_Neighbors = m_pCellSpace->GetNeighbors();
		}
		else
		{
			RegisterNeighbors(m_Agents[idx]);
		}		

		// Update Behaviors here
		m_pPrioritySteering->CalculateSteering(deltaT, m_Agents[idx]);
		m_Agents[idx]->Update(deltaT);

		// Trim to world
		if (m_TrimWorld)
		{
			m_Agents[idx]->TrimToWorld(m_WorldSize);
		}
	}

	// Agent to evade
	m_pSeekBehavior->CalculateSteering(deltaT, m_pAgentToEvade);
	m_pAgentToEvade->Update(deltaT);
	if (m_TrimWorld)
	{
		m_pAgentToEvade->TrimToWorld(m_WorldSize);
	}
}

void Flock::Render(float deltaT)
{
	// Render the flock
	/*for (int idx{}; idx < m_FlockSize; idx++)
	{
		m_Agents[idx]->Render(deltaT);
	}*/

	// Agent Variables
	const SteeringAgent* agentToDebug{ m_Agents[m_FlockSize - 1] };
	const Elite::Vector2 agentPos{ agentToDebug->GetPosition() };

	// NeighborhoodRect
	Elite::Rect neighborhoodRect{};
	neighborhoodRect.bottomLeft.x = agentPos.x - m_NeighborhoodRadius;
	neighborhoodRect.bottomLeft.y = agentPos.y - m_NeighborhoodRadius;
	neighborhoodRect.width = m_NeighborhoodRadius * 2;
	neighborhoodRect.height = m_NeighborhoodRadius * 2;

	// Show Debugging when enabled
	if (m_CanDebugRender)
	{
		// Agent Variables
		const Elite::Vector2 agentDirection{ agentToDebug->GetDirection() };
		const Elite::Vector2 agentVelocity{ agentToDebug->GetLinearVelocity() };

		// NeighborhoodRect
		float left = neighborhoodRect.bottomLeft.x;
		float bottom = neighborhoodRect.bottomLeft.y;
		float width = neighborhoodRect.width;
		float height = neighborhoodRect.height;

		std::vector<Elite::Vector2> rectPoints =
		{
			{ left , bottom  },
			{ left , bottom + height  },
			{ left + width , bottom + height },
			{ left + width , bottom  },
		};

		Elite::Polygon neighborhoodPolygon{ rectPoints };

		// Calculate SeparationDirection
		Elite::Vector2 desiredDirection{};
		Elite::Vector2 currNeighborDesiredDirection{};
		float currNeighborDistance{};

		for (int idx{}; idx < m_NrOfNeighbors; idx++)
		{
			currNeighborDesiredDirection = agentPos - m_Neighbors[idx]->GetPosition();
			currNeighborDistance = currNeighborDesiredDirection.Magnitude();

			currNeighborDesiredDirection.Normalize();

			desiredDirection += (1 / currNeighborDistance) * currNeighborDesiredDirection;

			// Draw Neighbors
			DEBUGRENDERER2D->DrawSolidCircle(m_Neighbors[idx]->GetPosition(), agentToDebug->GetRadius(), { 1,0 }, { 0,1,0 });
		}

		desiredDirection.Normalize();

		// Draw Neighborhood
		DEBUGRENDERER2D->DrawCircle(agentPos, m_NeighborhoodRadius, { 0,0,1 }, 0.9f);
		if(m_UseSpacePartitioning) DEBUGRENDERER2D->DrawPolygon(&neighborhoodPolygon, { 0,0,1 });
		
		// Draw average NeighborPos and NeighborVelocity
		DEBUGRENDERER2D->DrawPoint(GetAverageNeighborPos(), 5.f, { 1,1,0 });
		DEBUGRENDERER2D->DrawDirection(agentPos, GetAverageNeighborVelocity(), 5.f, { 1,1,0 });

		// Draw SeparationDirection
		DEBUGRENDERER2D->DrawDirection(agentPos, desiredDirection, 5.f, { 0,1,0 });

		// Draw AgentDirection
		DEBUGRENDERER2D->DrawDirection(agentPos, agentDirection, 5.f, { 1,0,0 });
	}
	
	// Render AgentToEvade
	m_pAgentToEvade->Render(deltaT);

	// Render CellSpace
	m_pCellSpace->RenderCells(m_ShowGrid, neighborhoodRect);
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
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

	ImGui::Text("Flocking");
	ImGui::Spacing();

	ImGui::Checkbox("Space Partitioning", &m_UseSpacePartitioning);
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Checkbox("Debug Rendering", &m_CanDebugRender);
	ImGui::Checkbox("Show Grid", &m_ShowGrid);
	ImGui::Checkbox("Trim World", &m_TrimWorld);
	if (m_TrimWorld)
	{
		ImGui::SliderFloat("Trim Size", &m_WorldSize, 0.f, 500.f, "%1.");
	}
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("Behavior Weights");
	ImGui::Spacing();

	ImGui::SliderFloat("Separation", &m_pBlendedSteering->GetWeightedBehaviorsRef()[0].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Alignment", &m_pBlendedSteering->GetWeightedBehaviorsRef()[1].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->GetWeightedBehaviorsRef()[2].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->GetWeightedBehaviorsRef()[3].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Seek", &m_pBlendedSteering->GetWeightedBehaviorsRef()[4].weight, 0.f, 1.f, "%.2");

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// Reset Neighbors
	m_NrOfNeighbors = 0;

	const Vector2 currentAgentPos{ pAgent->GetPosition() };
	
	Vector2 agentPos{};
	Vector2 agentToAgent{};

	// Loop through entire flock
	for (int idx{}; idx < m_FlockSize; idx++)
	{
		// If not the same agent
		if (m_Agents[idx] != pAgent)
		{
			agentPos = m_Agents[idx]->GetPosition();
			agentToAgent = agentPos - currentAgentPos;

			const bool isInsideRadius{ agentToAgent.MagnitudeSquared() < m_NeighborhoodRadius * m_NeighborhoodRadius };

			// If inside neighborRadius, be neighbor and adjust nrNeighbors;
			if (isInsideRadius)
			{
				m_Neighbors[m_NrOfNeighbors] = nullptr;
				m_Neighbors[m_NrOfNeighbors] = m_Agents[idx];
				++m_NrOfNeighbors;
			}
		}
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	std::vector<Vector2> neighborPosVector{};

	if (m_NrOfNeighbors == 0)
	{
		return {};
	}

	// Get all positions from current Neighbors
	int nrNeighborsChecked{};
	for (auto pNeighbor : m_Neighbors)
	{
		neighborPosVector.push_back(pNeighbor->GetPosition());
		
		++nrNeighborsChecked;
		if (nrNeighborsChecked == m_NrOfNeighbors)
		{
			break;
		}
	}
	
	// Calculate average position
	Vector2 averagePos{};
	for (size_t idx{}; idx < neighborPosVector.size(); idx++)
	{
		averagePos += neighborPosVector[idx];
	}
	averagePos /= float(m_NrOfNeighbors);

	return averagePos;
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	std::vector<Vector2> neighborVelocityVector{};

	if (m_NrOfNeighbors == 0)
	{
		return {};
	}

	// Get all positions from current Neighbors
	int nrNeighborsChecked{};
	for (auto pNeighbor : m_Neighbors)
	{
		neighborVelocityVector.push_back(pNeighbor->GetLinearVelocity());

		++nrNeighborsChecked;
		if (nrNeighborsChecked == m_NrOfNeighbors)
		{
			break;
		}
	}

	// Calculate average velocity
	Vector2 averageVelocity{};
	for (size_t idx{}; idx < neighborVelocityVector.size(); idx++)
	{
		averageVelocity += neighborVelocityVector[idx];
	}
	averageVelocity /= float(m_NrOfNeighbors);

	return averageVelocity;
}

void Flock::SetTarget_Seek(TargetData target)
{
	m_pSeekBehavior->SetTarget(target);
	m_pEvadeBehavior->SetTarget(m_pAgentToEvade->GetPosition());
}

float Flock::GetWorldTrimSize() const
{
	if (m_TrimWorld)
	{
		return m_WorldSize;
	}

	return 0;
}

float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}

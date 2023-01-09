#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
{	
	m_CellWidth = m_SpaceWidth / m_NrOfCols ;
	m_CellHeight = m_SpaceHeight / m_NrOfRows;

	// Create 2D vector
	for (int rowIdx{}; rowIdx < m_NrOfRows; rowIdx++)
	{
		for (int columnIdx{}; columnIdx < m_NrOfCols; columnIdx++)
		{
			const Cell newCell{ columnIdx * m_CellWidth,rowIdx * m_CellHeight,m_CellWidth,m_CellHeight };
			m_Cells.push_back(newCell);
		}
	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	const int cellIndex{ PositionToIndex(agent->GetPosition()) };
	m_Cells[cellIndex].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, Elite::Vector2 oldPos)
{
	// Compare old and new idx
	const int currIdx	{ PositionToIndex(agent->GetPosition()) };
	const int oldIdx	{ PositionToIndex(oldPos) };

	if (currIdx == oldIdx)
	{
		return;
	}

	// Remove out of old list and put in new list
	m_Cells[oldIdx].agents.remove(agent);
	m_Cells[currIdx].agents.push_back(agent);
}

void CellSpace::RegisterNeighbors(SteeringAgent* agent, float queryRadius)
{
	// Reset Neighbors
	m_NrOfNeighbors = 0;

	const Elite::Vector2 agentPos{ agent->GetPosition() };

	Elite::Rect neighborhoodRect{};
	neighborhoodRect.bottomLeft.x = agentPos.x - queryRadius;
	neighborhoodRect.bottomLeft.y = agentPos.y - queryRadius;
	neighborhoodRect.width = queryRadius * 2;
	neighborhoodRect.height = queryRadius * 2;

	Elite::Vector2 currentAgentPos;
	Elite::Vector2 agentToAgent{};

	for (size_t idx{}; idx < m_Cells.size(); ++idx)
	{
		// Check if neighborhoodRect isOverlapping with any Cells
		const bool isOverlapping{ Elite::IsOverlapping(m_Cells[idx].boundingBox,neighborhoodRect) };
		if (isOverlapping)
		{
			// If so, check if agents are in neighborhood radius
			for (SteeringAgent* agents : m_Cells[idx].agents)
			{
				if (agents != agent)
				{
					currentAgentPos = agents->GetPosition();
					agentToAgent = agentPos - currentAgentPos;

					const bool isInsideRadius{ agentToAgent.MagnitudeSquared() < queryRadius * queryRadius };

					// If inside neighborRadius, be neighbor and adjust nrNeighbors;
					if (isInsideRadius)
					{
						m_Neighbors[m_NrOfNeighbors] = nullptr;
						m_Neighbors[m_NrOfNeighbors] = agents;
						++m_NrOfNeighbors;
					}
				}
			}
		}
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : m_Cells)
		c.agents.clear();
}

void CellSpace::RenderCells(bool canDebugRender, const Elite::Rect& neighborhoodRect)const
{
	if (!canDebugRender)
	{
		return;
	}

	Elite::Polygon drawPolygon{};

	Elite::Vector2 stringDrawPos{};
	std::string stringToDraw{};
	
	std::vector<int> idxRememberVector{};

	// Loop through all cells
	for (size_t idx{}; idx < m_Cells.size(); idx++)
	{
		drawPolygon = Elite::Polygon{ m_Cells[idx].GetRectPoints() };

		// StringDraw
		stringDrawPos = m_Cells[idx].GetRectPoints()[0];
		stringDrawPos.y += m_CellHeight;
		stringToDraw = std::to_string(m_Cells[idx].agents.size());

		// Remember idx, to draw overlapped Cell later
		const bool isOverlapping{ Elite::IsOverlapping(m_Cells[idx].boundingBox, neighborhoodRect) };
		if (isOverlapping && canDebugRender)
		{
			idxRememberVector.push_back(idx);
		}
		
		// DEBUG
		DEBUGRENDERER2D->DrawPolygon(&drawPolygon, { 1,0,0 });
		DEBUGRENDERER2D->DrawString(stringDrawPos, stringToDraw.data());
	}

	// Draw overlapped Cells
	for (size_t idx{}; idx < idxRememberVector.size(); idx++)
	{
		drawPolygon = Elite::Polygon{ m_Cells[idxRememberVector[idx]].GetRectPoints() };
		DEBUGRENDERER2D->DrawPolygon(&drawPolygon, { 0,1,1 });
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	// Cols
	float t{ pos.x / m_CellWidth };
	int colIdx{ int(t) };

	// Limit idx
	if (colIdx > m_NrOfCols)	colIdx = m_NrOfCols;
	if (colIdx < 0)				colIdx = 0;

	// Rows
	t = pos.y / m_CellHeight;
	int rowIdx{ int(t) };

	// Limit idx
	if (rowIdx > m_NrOfRows)	rowIdx = m_NrOfRows;
	if (rowIdx < 0)				rowIdx = 0;

	// Final
	int cellIdx{ m_NrOfCols * rowIdx + colIdx };

	// Limit idx
	if (cellIdx >= int(m_Cells.size())) cellIdx = int(m_Cells.size() - 1);

	return cellIdx;
}
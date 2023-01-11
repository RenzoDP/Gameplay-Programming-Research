#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon; 
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create nodes
	int nodeIndex{};
	for (const auto& line : m_pNavMeshPolygon->GetLines())
	{
		// Check if is connected to another triangle
		if (m_pNavMeshPolygon->GetTrianglesFromLineIndex(line->index).size() > 1)
		{
			const Vector2 middlePos{ (line->p2 + line->p1) / 2 };

			// Create NavGraphNode to the graph
			NavGraphNode* pNewGraphNode = new NavGraphNode{ nodeIndex,line->index,middlePos };
			this->AddNode(pNewGraphNode);

			++nodeIndex;
		}
	}

	//2. Create connections now that every node is created
	std::vector<NavGraphNode*> connectionNodes{};

	for (const auto& triangle : m_pNavMeshPolygon->GetTriangles())
	{
		// Loop over line indexes
		for (size_t idx{}; idx < triangle->metaData.IndexLines.size(); ++idx)
		{
			// Check if valid node for that lineIdx exists
			const int nodeIdx{ this->GetNodeIdxFromLineIdx(triangle->metaData.IndexLines[idx]) };
			if (nodeIdx != invalid_node_index)
			{		
				// Add node to vector for connection later
				connectionNodes.push_back(this->GetNode(nodeIdx));
			}
		}

		int fromIdx{}, toIdx{};

		// Create connections, but check if unique first
		if (connectionNodes.size() == 2)
		{
			fromIdx = connectionNodes[0]->GetIndex();
			toIdx = connectionNodes[1]->GetIndex();

			if (this->IsUniqueConnection(fromIdx, toIdx))
			{
				GraphConnection2D* newConnection = new GraphConnection2D{ fromIdx , toIdx };
				this->AddConnection(newConnection);
			}
		}
		else if(connectionNodes.size() == 3)
		{
			// First connection
			fromIdx = connectionNodes[0]->GetIndex();
			toIdx = connectionNodes[1]->GetIndex();

			if (this->IsUniqueConnection(fromIdx, toIdx))
			{
				GraphConnection2D* newFirstConnection = new GraphConnection2D{ fromIdx,toIdx };
				this->AddConnection(newFirstConnection);
			}

			// Second connection
			fromIdx = connectionNodes[1]->GetIndex();
			toIdx = connectionNodes[2]->GetIndex();

			if (this->IsUniqueConnection(fromIdx, toIdx))
			{
				GraphConnection2D* newSecondConnection = new GraphConnection2D{ fromIdx , toIdx };
				this->AddConnection(newSecondConnection);
			}
			
			// Third connection
			fromIdx = connectionNodes[2]->GetIndex();
			toIdx = connectionNodes[0]->GetIndex();

			if (this->IsUniqueConnection(fromIdx, toIdx))
			{
				GraphConnection2D* newThirdConnection = new GraphConnection2D{ fromIdx , toIdx };
				this->AddConnection(newThirdConnection);
			}
		}

		connectionNodes.clear();
	}

	//3. Set the connections cost to the actual distance
	this->SetConnectionCostsToDistance();
}


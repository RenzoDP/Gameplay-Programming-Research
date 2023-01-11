#pragma once
#include <vector>
#include <iostream>
#include "framework/EliteMath/EMath.h"
#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EBFS.h"

namespace Elite
{
	class NavMeshPathfinding
	{
	public:
		static std::vector<Vector2> FindPath(Vector2 startPos, Vector2 endPos, NavGraph* pNavGraph, std::vector<Vector2>& debugNodePositions, std::vector<Portal>& debugPortals)
		{
			//Create the path to return
			std::vector<Vector2> finalPath{};

			//Get the start and endTriangle
			Elite::Polygon* pNavMeshPolygon { pNavGraph->GetNavMeshPolygon() };

			const Triangle* pStartTriangle{ pNavMeshPolygon->GetTriangleFromPosition(startPos) };
			const Triangle* pEndTriangle{ pNavMeshPolygon->GetTriangleFromPosition(endPos) };

			//We have valid start/end triangles and they are not the same
			const bool trianglesDontExist{ pStartTriangle == nullptr || pEndTriangle == nullptr };
			const bool trianglesAreTheSame{ pStartTriangle == pEndTriangle };
			if (trianglesDontExist || trianglesAreTheSame)
			{
				return {};
			}
			
			//=> Start looking for a path
			//Copy the graph
			auto clonedGraph{ pNavGraph->Clone() };
			
			//Create extra node for the Start Node (Agent's position
			NavGraphNode* pStartNode = new NavGraphNode{ clonedGraph->GetNextFreeNodeIndex(),-1,startPos };
			clonedGraph->AddNode(pStartNode);
			
			std::vector<int> nodeIdxVector{};

			// Loop over lineIdx to get all the available graphNodes
			for (const auto& lineIdx : pStartTriangle->metaData.IndexLines)
			{
				int nodeIdx{ pNavGraph->GetNodeIdxFromLineIdx(lineIdx) };
				if (nodeIdx != invalid_node_index)
				{
					nodeIdxVector.push_back(nodeIdx);
				}
			}

			// Create connections from startNode to graphNodes
			for (const int index : nodeIdxVector)
			{
				GraphConnection2D* pNewGraphConnection = new GraphConnection2D{ pStartNode->GetIndex(),index };

				// Set cost to distance between nodes
				pNewGraphConnection->SetCost(abs(Distance(pStartNode->GetPosition(), clonedGraph->GetNode(index)->GetPosition())));

				clonedGraph->AddConnection(pNewGraphConnection);
			}


			//Create extra node for the endNode
			NavGraphNode* pEndNode = new NavGraphNode{ clonedGraph->GetNextFreeNodeIndex(),-1,endPos };
			clonedGraph->AddNode(pEndNode);

			nodeIdxVector.clear();

			// Loop over lineIdx to get all the available graphNodes
			for (const auto& lineIdx : pEndTriangle->metaData.IndexLines)
			{
				int nodeIdx{ pNavGraph->GetNodeIdxFromLineIdx(lineIdx) };
				if (nodeIdx != invalid_node_index)
				{
					nodeIdxVector.push_back(nodeIdx);
				}
			}

			// Create connections from endNode to graphNodes
			for (const int index : nodeIdxVector)
			{
				GraphConnection2D* pNewGraphConnection = new GraphConnection2D{ pEndNode->GetIndex(),index };

				// Set cost to distance between nodes
				pNewGraphConnection->SetCost(abs(Distance(pEndNode->GetPosition(), clonedGraph->GetNode(index)->GetPosition())));

				clonedGraph->AddConnection(pNewGraphConnection);
			}
			
			//Run A star on new graph
			const Elite::Heuristic heuristic{ HeuristicFunctions::Chebyshev };
			auto pathFinder = AStar<NavGraphNode, GraphConnection2D>(clonedGraph.get(), heuristic);
			
			// Store nodes in vector for the portals
			std::vector<NavGraphNode*> nodeVector{};

			const auto AStarPath = pathFinder.FindPath(pStartNode, pEndNode);
			for (const auto& node : AStarPath)
			{
				finalPath.push_back(node->GetPosition());
				nodeVector.push_back(node);
			}

			//OPTIONAL BUT ADVICED: Debug Visualisation
			debugNodePositions = finalPath;

			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			auto portals = SSFA::FindPortals(nodeVector, pNavMeshPolygon);
			finalPath = SSFA::OptimizePortals(portals);

			//OPTIONAL BUT ADVICED: Debug Visualisation
			debugPortals = portals;

			return finalPath;
		}
	};
}

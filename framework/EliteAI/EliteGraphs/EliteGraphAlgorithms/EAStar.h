#pragma once
#include "framework/EliteAI/EliteNavigation/ENavigation.h"

namespace Elite
{

	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		// StartVariables
		std::vector<T_NodeType*> path;
		std::vector<NodeRecord> openList;
		std::vector<NodeRecord> closedList;
		NodeRecord currentRecord;

		// StartRecord
		NodeRecord startRecord;
		startRecord.pNode = pStartNode;
		startRecord.pConnection = nullptr;
		startRecord.costSoFar = 0;
		startRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);

		openList.push_back(startRecord);

		bool goToNextConnection{ false };
		NodeRecord existingRecord;
		existingRecord.pNode = nullptr;

		bool hasSameNode{};
		bool hasSameConnection{};

		// Start the loop
		while (openList.empty() == false)
		{
			// Get lowestCost node from openList
			currentRecord = *std::min_element(openList.begin(), openList.end());

			// Check if lowestCost node is endNode
			if (currentRecord.pNode == pGoalNode)
			{
				break;
			}

			// Get all connections
			for (auto& connection : m_pGraph->GetNodeConnections(currentRecord.pNode))
			{
				// Get node that connections is pointing to
				T_NodeType* pNextNode{ m_pGraph->GetNode(connection->GetTo()) };


				//---------------//
				//--First Check--//
				//---------------//

				// Check if node is already on closedList
				for (auto& nodeRecord : closedList)
				{
					if (pNextNode == nodeRecord.pNode)
					{
						// If connection is cheaper, replace with old connection
						if (currentRecord.estimatedTotalCost < nodeRecord.estimatedTotalCost)
						{
							existingRecord = nodeRecord;
						}
						// Else just leave it
						else
						{
							goToNextConnection = true;
						}

						break;
					}
				}

				// Remove from closedList if necesarry
				if (existingRecord.pNode)
				{
					for (size_t idx{}; idx < closedList.size(); ++idx)
					{
						hasSameNode = closedList[idx].pNode == currentRecord.pNode;
						hasSameConnection = closedList[idx].pConnection == currentRecord.pConnection;

						if (hasSameNode && hasSameConnection)
						{
							closedList.erase(closedList.begin() + idx);
							break;
						}
					}

					// Reset variables
					existingRecord = {};
					existingRecord.pNode = nullptr;
				}

				// Go next if needed
				if (goToNextConnection)
				{
					goToNextConnection = false;
					continue;
				}


				//----------------//
				//--Second Check--//
				//----------------//

				// Check if node is already on openList
				for (auto& nodeRecord : openList)
				{
					if (pNextNode == nodeRecord.pNode)
					{
						// If connection is cheaper, replace with old connection
						if (currentRecord.estimatedTotalCost < nodeRecord.estimatedTotalCost)
						{
							existingRecord = nodeRecord;
						}
						// Else just leave it
						else
						{
							goToNextConnection = true;
						}

						break;
					}
				}

				// Remove from openList if necesarry
				if (existingRecord.pNode)
				{
					for (size_t idx{}; idx < openList.size(); ++idx)
					{
						hasSameNode = openList[idx].pNode == currentRecord.pNode;
						hasSameConnection = openList[idx].pConnection == currentRecord.pConnection;

						if (hasSameNode && hasSameConnection)
						{
							openList.erase(openList.begin() + idx);
							break;
						}
					}

					// Reset variables
					existingRecord = {};
					existingRecord.pNode = nullptr;
				}

				// Go next if needed
				if (goToNextConnection)
				{
					goToNextConnection = false;
					continue;
				}


				//-------------------//
				//--Add to openList--//
				//-------------------//

				NodeRecord newRecord;
				newRecord.pNode = pNextNode;
				newRecord.pConnection = connection;
				newRecord.costSoFar = currentRecord.costSoFar + connection->GetCost();
				newRecord.estimatedTotalCost = GetHeuristicCost(newRecord.pNode, pGoalNode) + newRecord.costSoFar;

				openList.push_back(newRecord);
			}

			// Remove from openList and add to closedList
			for (size_t idx{}; idx < openList.size(); ++idx)
			{
				hasSameNode = openList[idx].pNode == currentRecord.pNode;
				hasSameConnection = openList[idx].pConnection == currentRecord.pConnection;

				if (hasSameNode && hasSameConnection)
				{
					openList.erase(openList.begin() + idx);
					break;
				}
			}
			closedList.push_back(currentRecord);
		}


		//--------------------//
		//--Reconstruct Path--//
		//--------------------//

		// Do until currentNode is startNode
		while (currentRecord.pNode != pStartNode)
		{
			// Add node to path
			path.push_back(currentRecord.pNode);

			for (auto& records : closedList)
			{
				// TrackBack
				if (records.pNode == m_pGraph->GetNode(currentRecord.pConnection->GetFrom()))
				{
					currentRecord = records;
					break;
				}
			}
		}

		path.push_back(pStartNode);
		std::reverse(path.begin(), path.end());
		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}
#pragma once
#include <stack>

namespace Elite
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	template <class T_NodeType, class T_ConnectionType>
	class EulerianPath
	{
	public:

		EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		Eulerianity IsEulerian() const;
		std::vector<T_NodeType*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(int startIdx, std::vector<bool>& visited) const;
		bool IsConnected() const;
		void GetNodePath(std::shared_ptr<IGraph<T_NodeType, T_ConnectionType>> copyGraph, int startIdx, std::vector<T_NodeType*>& path, std::vector<bool>& visited) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template<class T_NodeType, class T_ConnectionType>
	inline EulerianPath<T_NodeType, T_ConnectionType>::EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{
	}

	template<class T_NodeType, class T_ConnectionType>
	inline Eulerianity EulerianPath<T_NodeType, T_ConnectionType>::IsEulerian() const
	{

		// If the graph is not connected, there can be no Eulerian Trail
		if (IsConnected() == false)
		{
			return Eulerianity::notEulerian;
		}

		// Count nodes with odd degree 
		auto nodes = m_pGraph->GetAllNodes();
		int oddCount = 0;

		for (auto n : nodes)
		{
			auto connections = m_pGraph->GetNodeConnections(n);
			if (connections.size() & 1)						// You can also do: connections.size() & 1
			{
				++oddCount;
			}
		}

		// A connected graph with more than 2 nodes with an odd degree (an odd amount of connections) is not Eulerian
		if (oddCount > 2)
		{
			return Eulerianity::notEulerian;
		}

		// A connected graph with exactly 2 nodes with an odd degree is Semi-Eulerian (unless there are only 2 nodes)
		// An Euler trail can be made, but only starting and ending in these 2 nodes
		if (oddCount == 2 && nodes.size() != 2)
		{
			return Eulerianity::semiEulerian;
		}

		// A connected graph with no odd nodes is Eulerian
		return Eulerianity::eulerian;
	}

	template<class T_NodeType, class T_ConnectionType>
	inline std::vector<T_NodeType*> EulerianPath<T_NodeType, T_ConnectionType>::FindPath(Eulerianity& eulerianity) const
	{
		// Get a copy of the graph because this algorithm involves removing edges
		auto graphCopy = m_pGraph->Clone();
		auto path = std::vector<T_NodeType*>();
		int nrOfNodes = graphCopy->GetNrOfNodes();

		// Check if there can be an Euler path
		// If this graph is not eulerian, return the empty path
		// Else we need to find a valid starting index for the algorithm
		if (eulerianity == Eulerianity::notEulerian)
		{
			return path;
		}

		// find a valid starting node that has connections
		auto nodes = m_pGraph->GetAllNodes();
		vector<bool> visited(nrOfNodes, false);

		// If no connections, return
		if (nodes.size() > 1 && m_pGraph->GetAllConnections().size() == 0)
		{
			return path;
		}

		int startIdx = invalid_node_index;
		for (auto n : nodes)
		{
			// Has connections and is uneven
			auto connections = m_pGraph->GetNodeConnections(n);
			if (connections.size() != 0 && connections.size() % 2 == 1)
			{
				startIdx = n->GetIndex();
				break;
			}
		}
		// if no valid node could be found, pick first one
		if (startIdx == invalid_node_index)
		{
			startIdx = nodes[0]->GetIndex();
		}

		// Start algorithm loop
		std::stack<int> nodeStack;
		int currentNodeIdx{ startIdx };
		T_ConnectionType* currentConnection;

		// Loop while Node still has connections or stack is still not empty
		bool stillHasConnections{ graphCopy->GetNodeConnections(currentNodeIdx).size() > 0 };
		bool stackNotEmpty{ nodeStack.size() > 0 };
		while (stillHasConnections || stackNotEmpty)
		{
			// If node has neighbors
			if (graphCopy->GetNodeConnections(currentNodeIdx).size() > 0)
			{
				// Add node to stack
				nodeStack.push(currentNodeIdx);

				// Take a neighbor
				currentConnection = graphCopy->GetNodeConnections(currentNodeIdx).front();

				// Set neighbor as new node
				currentNodeIdx = currentConnection->GetTo();

				// Remove connection
				graphCopy->RemoveConnection(currentConnection);
			}
			else
			{
				// Add currentNode to path
				path.push_back(m_pGraph->GetNode(currentNodeIdx));

				// Take last stack node as current one
				currentNodeIdx = nodeStack.top();

				// Remove node from stack
				nodeStack.pop();
			}

			// Re-assign bools
			stillHasConnections = graphCopy->GetNodeConnections(currentNodeIdx).size() > 0;
			stackNotEmpty = nodeStack.size() > 0;
		}

		// Add-in first node
		path.push_back(m_pGraph->GetNode(startIdx));

		std::reverse(path.begin(), path.end()); // reverses order of the path
		return path;
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void EulerianPath<T_NodeType, T_ConnectionType>::VisitAllNodesDFS(int startIdx, std::vector<bool>& visited) const
	{
		// mark the visited node
		visited[startIdx] = true;

		// recursively visit any valid connected nodes that were not visited before
		for (T_ConnectionType* connection : m_pGraph->GetNodeConnections(startIdx))
		{
			if (visited[connection->GetTo()] == false)
			{
				VisitAllNodesDFS(connection->GetTo(), visited);
			}
		}

	}

	template<class T_NodeType, class T_ConnectionType>
	inline bool EulerianPath<T_NodeType, T_ConnectionType>::IsConnected() const
	{
		auto nodes = m_pGraph->GetAllNodes();
		vector<bool> visited(m_pGraph->GetNrOfNodes(), false);

		// find a valid starting node that has connections
		int connectedIdx = invalid_node_index;

		if (nodes.size() > 1 && m_pGraph->GetAllConnections().size() == 0)
		{
			return false;
		}

		for (auto n : nodes)
		{
			auto connections = m_pGraph->GetNodeConnections(n);
			if (connections.size() != 0)
			{
				connectedIdx = n->GetIndex();
				break;
			}
		}
		// if no valid node could be found, return false
		if (connectedIdx == invalid_node_index)
		{
			return false;
		}

		// start a depth-first-search traversal from the node that has at least one connection
		VisitAllNodesDFS(connectedIdx, visited);

		// if a node was never visited, this graph is not connected
		for (auto n : nodes)
		{
			if (visited[n->GetIndex()] == false)
			{
				return false;
			}
		}

		return true;
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void EulerianPath<T_NodeType, T_ConnectionType>::GetNodePath(std::shared_ptr<IGraph<T_NodeType, T_ConnectionType>> copyGraph, int startIdx, std::vector<T_NodeType*>& path, std::vector<bool>& visited) const
	{
		//std::vector< T_ConnectionType*> connectionsVector;

		// mark the visited node
		visited[startIdx] = true;

		// recursively visit any valid connected nodes that were not visited before
		for (T_ConnectionType* connection : copyGraph->GetNodeConnections(startIdx))
		{
			if (visited[connection->GetTo()] == false)
			{
				GetNodePath(copyGraph,connection->GetTo(), path, visited);
			}

			//connectionsVector.push_back(connection);
			copyGraph->RemoveConnection(connection);
		}

		/*const int startNumber{ (int)copyGraph->GetNodeConnections(startIdx).size() };
		for (int idx{}; idx < startNumber; ++idx)
		{
			if (copyGraph->GetNodeConnections(startIdx).)
			{

			}

			if (visited[connection->GetTo()] == false)
			{
				GetNodePath(copyGraph, connection->GetTo(), path, visited);
			}

			copyGraph->RemoveConnection(connection);
		}*/

		for (T_ConnectionType* connection; connection != nullptr; ++connection)
		{

		}

		/*for (size_t idx{}; idx < connectionsVector.size(); ++idx)
		{
			copyGraph->RemoveConnection(connectionsVector[idx]);
		}*/
		

		if (copyGraph->GetNodeConnections(startIdx).size() == 0)
		{
			path.push_back(m_pGraph->GetNode(startIdx));
			return;
		}
	}
}
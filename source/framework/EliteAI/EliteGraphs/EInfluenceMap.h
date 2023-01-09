#pragma once
#include "EIGraph.h"
#include "EGraphNodeTypes.h"
#include "EGraphConnectionTypes.h"

namespace Elite
{
	template<class T_GraphType>
	class InfluenceMap final : public T_GraphType
	{
	public:
		InfluenceMap(bool isDirectional): T_GraphType(isDirectional) {}
		void InitializeBuffer() { m_InfluenceDoubleBuffer = std::vector<float>(m_Nodes.size()); }
		void PropagateInfluence(float deltaTime);

		void SetInfluenceAtPosition(Elite::Vector2 pos, float influence);

		void Render() const {}
		void SetNodeColorsBasedOnInfluence();

		float GetMomentum() const { return m_Momentum; }
		void SetMomentum(float momentum) { m_Momentum = momentum; }

		float GetDecay() const { return m_Decay; }
		void SetDecay(float decay) { m_Decay = decay; }

		float GetPropagationInterval() const { return m_PropagationInterval; }
		void SetPropagationInterval(float propagationInterval) { m_PropagationInterval = propagationInterval; }

		void SetChangedGraph(bool changedGraph) { m_ChangedGraph = changedGraph; };
		void SetBlockedNodes(std::vector<Elite::InfluenceNode*> blockedNodes) { m_BlockedNodes = blockedNodes; };

	protected:
		virtual void OnGraphModified(bool nrOfNodesChanged, bool nrOfConnectionsChanged) override;

	private:
		Elite::Color m_NegativeColor{ 1.f, 0.2f, 0.f};
		Elite::Color m_NeutralColor{ 0.3f, 0.3f, 0.3f };
		Elite::Color m_PositiveColor{ 0.f, 0.2f, 1.f};

		float m_MaxAbsInfluence = 100.f;

		float m_Momentum = 0.8f; // a higher momentum means a higher tendency to retain the current influence
		float m_Decay = 0.1f; // determines the decay in influence over distance

		float m_PropagationInterval = .05f; //in Seconds
		float m_TimeSinceLastPropagation = 0.0f;

		std::vector<float> m_InfluenceDoubleBuffer;

		// Own Stuff
		bool m_ChangedGraph{ false };
		Elite::Color m_SeparatedColor{ 0.7f,0.7f,0.7f };

		std::vector<Elite::InfluenceNode*> m_BlockedNodes{};
	};

	template <class T_GraphType>
	void InfluenceMap<T_GraphType>::PropagateInfluence(float deltaTime)
	{
		// TODO: implement
		m_TimeSinceLastPropagation += deltaTime;
		if (m_TimeSinceLastPropagation > m_PropagationInterval)
		{
			m_TimeSinceLastPropagation -= m_PropagationInterval;

			// Clear buffer
			m_InfluenceDoubleBuffer.clear();

			// Loop over the nodes
			for (const auto& currentNode : m_Nodes)
			{
				float highestInfluence{};
				float connectionCostFromHighestInfluenceNode{};

				// Loop over the connections from currentNode
				const auto nodeConnections{ GetNodeConnections(currentNode) };
				for (const auto& currentConnection : nodeConnections)
				{
					const auto getToNode{ GetNode(currentConnection->GetTo()) };
					const float toNodeInfluence{ getToNode->GetInfluence() };

					// If influence from node is higher then highest one, replace
					if (abs(toNodeInfluence) > abs(highestInfluence))
					{
						highestInfluence = toNodeInfluence;
						connectionCostFromHighestInfluenceNode = currentConnection->GetCost();
					}
				}

				// Apply formula
				const float newInfluence{ highestInfluence * expf(-connectionCostFromHighestInfluenceNode * m_Decay) };
				float desiredInfluence{ Lerp(currentNode->GetInfluence(), newInfluence, m_Momentum) };
				
				// If influence is more then max, clamp
				if (m_MaxAbsInfluence < abs(desiredInfluence))
				{
					// Clamp to either negative or positive maxInfluence
					if (desiredInfluence < 0)
					{
						desiredInfluence = -m_MaxAbsInfluence;
					}
					else
					{
						desiredInfluence = m_MaxAbsInfluence;
					}
				}

				// Put in buffer
				m_InfluenceDoubleBuffer.push_back(desiredInfluence);
			}


			// Loop over the nodes
			for (size_t idx{}; idx < m_Nodes.size(); ++idx)
			{
				// Set Influence
				m_Nodes[idx]->SetInfluence(m_InfluenceDoubleBuffer[idx]);
			}
		}
	}

	template <class T_GraphType>
	inline void InfluenceMap<T_GraphType>::SetInfluenceAtPosition(Elite::Vector2 pos, float influence)
	{
		auto idx = GetNodeIdxAtWorldPos(pos);
		if (IsNodeValid(idx))
			GetNode(idx)->SetInfluence(influence);
	}

	template<class T_GraphType>
	inline void InfluenceMap<T_GraphType>::SetNodeColorsBasedOnInfluence()
	{
		const float half = .5f;

		for (auto& pNode : m_Nodes)
		{
			// Ignore nodes on blockedList
			bool isOnBlockedNodes{ false };
			for (const auto& currentBlockedNode : m_BlockedNodes)
			{
				if (pNode->GetIndex() == currentBlockedNode->GetIndex())
				{
					isOnBlockedNodes = true;
				}
			}

			if(isOnBlockedNodes) continue;

			// Framework

			Color nodeColor{};
			float influence = pNode->GetInfluence();
			float relativeInfluence = abs(influence) / m_MaxAbsInfluence;

			if (influence < 0)
			{
				nodeColor = Elite::Color{
				Lerp(m_NeutralColor.r, m_NegativeColor.r, relativeInfluence),
				Lerp(m_NeutralColor.g, m_NegativeColor.g, relativeInfluence),
				Lerp(m_NeutralColor.b, m_NegativeColor.b, relativeInfluence)
				};
			}
			else
			{
				nodeColor = Elite::Color{
				Lerp(m_NeutralColor.r, m_PositiveColor.r, relativeInfluence),
				Lerp(m_NeutralColor.g, m_PositiveColor.g, relativeInfluence),
				Lerp(m_NeutralColor.b, m_PositiveColor.b, relativeInfluence)
				};
			}

			pNode->SetColor(nodeColor);
		}

		//// If Graph wasn't changed, return
		//if (m_ChangedGraph == false) return;

		//// Else, change Color of separate Nodes
		//for (size_t nodeIdx{}; nodeIdx < m_Nodes.size(); ++nodeIdx)
		//{
		//	const auto nodeConnections{ m_Connections[nodeIdx] };
		//	
		//	// If no connections, change Color
		//	const bool hasNoConnections
		//}
	}

	template<class T_GraphType>
	inline void InfluenceMap<T_GraphType>::OnGraphModified(bool nrOfNodesChanged, bool nrOfConnectionsChanged)
	{
		InitializeBuffer();
	}
}
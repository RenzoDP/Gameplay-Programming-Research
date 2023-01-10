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

		std::vector<Elite::InfluenceNode*> GetBlockedNodes() const { return m_BlockedNodes; };
		void SetBlockedNodes(std::vector<Elite::InfluenceNode*> blockedNodes) { m_BlockedNodes = blockedNodes; };

		bool GetShouldEndAlgorithm() const { return m_ShouldEndAlgorithm; };
		void SetShouldEndAlgorithm(bool shouldEndAlgorithm) { m_ShouldEndAlgorithm = shouldEndAlgorithm; };

		float GetMaxHeat() const { return m_MaxHeat / 20.f; };
		void SetMaxHeat(float maxHeat) { m_MaxHeat = maxHeat * 20.f; };

		float GetMaxAmountOfHeatedCells() const { return m_MaxAmountOfHeatedCells / 30.f; };
		void SetMaxAmountOfHeatedCells(float maxAmountOfHeatedCells) { m_MaxAmountOfHeatedCells = int(maxAmountOfHeatedCells * 30.f); };

		float GetMaxPropagationSteps() const { return m_MaxPropagationSteps / 30.f; };
		void SetMaxPropagationSteps(float maxPropagationSteps) { m_MaxPropagationSteps = int(maxPropagationSteps * 30.f); };

	protected:
		virtual void OnGraphModified(bool nrOfNodesChanged, bool nrOfConnectionsChanged) override;

	private:
		Elite::Color m_NegativeColor{ 1.f, 0.2f, 0.f};
		Elite::Color m_NeutralColor{ 0.3f, 0.3f, 0.3f };
		Elite::Color m_PositiveColor{ 0.f, 0.2f, 1.f};

		float m_Momentum = 0.8f; // a higher momentum means a higher tendency to retain the current influence
		float m_Decay = 0.1f; // determines the decay in influence over distance

		float m_PropagationInterval = .05f; //in Seconds
		float m_TimeSinceLastPropagation = 0.0f;

		std::vector<float> m_InfluenceDoubleBuffer;

		// Own Stuff
		std::vector<Elite::InfluenceNode*> m_BlockedNodes{};

		bool m_ShouldEndAlgorithm{ false };
		int m_CurrentPropagationSteps{};

		float m_MaxHeat{ 9.f };
		int m_MaxAmountOfHeatedCells{ 15 };
		int m_MaxPropagationSteps{ 20 };
	};

	template <class T_GraphType>
	void InfluenceMap<T_GraphType>::PropagateInfluence(float deltaTime)
	{
		// TODO: implement
		m_TimeSinceLastPropagation += deltaTime;
		if (m_TimeSinceLastPropagation > m_PropagationInterval)
		{
			m_TimeSinceLastPropagation -= m_PropagationInterval;

			// Plus PropagationSteps
			if (m_ShouldEndAlgorithm)
			{
				m_CurrentPropagationSteps = 0;
			}
			else
			{
				++m_CurrentPropagationSteps;
			}

			// Remove Connections off blockedNodes
			for (const auto& currentBlockedNode : m_BlockedNodes)
			{
				RemoveConnectionsToAdjacentNodes(currentBlockedNode->GetIndex());
			}

			// Keep count of cellsHeated
			int cellsHeated{};

			// Loop over the Nodes
			for (const auto& currentNode : m_Nodes)
			{
				const bool isCold{ currentNode->GetInfluence() <= 0.f };			

				// Check if Neighbor is Heated or Blocked
				// --------------------------------------

				bool neighborIsHeated{ false };

				// Loop over the connections from currentNode
				const auto nodeConnections{ GetNodeConnections(currentNode) };
				for (const auto& currentConnection : nodeConnections)
				{
					const auto getToNode{ GetNode(currentConnection->GetTo()) };
					const float toNodeInfluence{ getToNode->GetInfluence() };

					// Check if Neighbor has maxInfluence (= Heated)
					const bool isHeated{ toNodeInfluence >= m_MaxHeat };
					if (isHeated)
					{
						neighborIsHeated = true;
						break;
					}
				}

				// Calculate desiredHeat
				// ---------------------
				float desiredHeat{};
				
				// If currentNode hasn't been effected yet, put on maxHeat
				if (isCold && neighborIsHeated && m_ShouldEndAlgorithm == false)
				{
					desiredHeat = m_MaxHeat;
					++cellsHeated;
				}
				// If have been effected already, lower Influence
				else if (isCold == false)
				{
					const float currentNodeInfluence{ currentNode->GetInfluence() };
					desiredHeat = currentNodeInfluence - 1;
					desiredHeat =Elite::Clamp(desiredHeat, 0.f, m_MaxHeat);
				}

				// Put in Buffer
				// -------------
				m_InfluenceDoubleBuffer[currentNode->GetIndex()] = desiredHeat;
			}

			// Re-Add Connections off blockedNodes
			for (const auto& currentBlockedNode : m_BlockedNodes)
			{
				AddConnectionsToAdjacentCells(currentBlockedNode->GetIndex());
			}

			// Loop over the nodes
			for (size_t idx{}; idx < m_Nodes.size(); ++idx)
			{
				// Set Influence
				m_Nodes[idx]->SetInfluence(m_InfluenceDoubleBuffer[idx]);
			}

			// Check if should stop Algorithm
			const bool maxHeatedCellsReached{ cellsHeated >= m_MaxAmountOfHeatedCells };
			const bool maxPropagationStepsReached{ m_CurrentPropagationSteps >= m_MaxPropagationSteps };
			const bool noMoreCellsToHeat{ cellsHeated == 0 };

			if (maxPropagationStepsReached || maxHeatedCellsReached || noMoreCellsToHeat)
			{
				m_ShouldEndAlgorithm = true;
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
			float relativeInfluence = abs(influence) / m_MaxHeat;

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
	}

	template<class T_GraphType>
	inline void InfluenceMap<T_GraphType>::OnGraphModified(bool nrOfNodesChanged, bool nrOfConnectionsChanged)
	{
		if (nrOfConnectionsChanged == 0)
		{
			InitializeBuffer();
		}
	}
}
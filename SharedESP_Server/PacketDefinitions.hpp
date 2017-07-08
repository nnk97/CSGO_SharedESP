#pragma once

#include <stdint.h>	// For uint16_t

namespace Server
{
	enum PacketType : uint16_t
	{
		None,
		Update,
		Query
	};

	struct UpdateEntityPacket_t
	{
	public:
		int m_Index;
		float m_SimulationTime;
		float m_Position[3];
		bool m_Crouching;

		template <typename Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & m_Index;
			ar & m_SimulationTime;
			ar & m_Position[0];
			ar & m_Position[1];
			ar & m_Position[2];
			ar & m_Crouching;
		}
	};

	struct PacketHeader_t
	{
	public:
		PacketType m_Type;
		uint32_t m_ServerHash;
		uint32_t m_SizeParam;

		template <typename Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & m_Type;
			ar & m_ServerHash;
			ar & m_SizeParam;
		}
	};
}

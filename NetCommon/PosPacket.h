#pragma once
#include "Packet.h"

class PosPacket : public IPacket
{
public:
	PosPacket();
	~PosPacket();

	std::string UserID;
	int PosX;
	int PosY;

	PacketType GetType() override;

	void Parse(std::string InString) override;

	std::string ToString() override;

	int Length() override;
};


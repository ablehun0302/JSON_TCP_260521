#pragma once
#include "Packet.h"
class ChatPacket : public IPacket
{
public:
	ChatPacket();
	~ChatPacket();

	std::string UserID;
	std::string Icon;
	std::string MoveCode;

	// Inherited via IPacket
	PacketType GetType() override;

	void Parse(std::string InString) override;

	std::string ToString() override;

	int Length() override;

};


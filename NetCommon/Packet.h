#pragma once
#include "pch.h"

#define HEADER_SIZE		8

enum class PacketType
{
	Chat = 0,
	Pos
};

struct Header
{
	unsigned int PacketSize;
	unsigned int PacketAmount;
};

struct PosData
{
	char Icon;
	int PosX;
	int PosY;
};

class IPacket
{
public:
	virtual PacketType GetType() = 0;
	virtual void Parse(std::string InString) = 0;
	virtual std::string ToString() = 0;
	virtual int Length() = 0;

	rapidjson::Document JSONDocument;
};


#pragma once
#include "pch.h"
enum class PacketType
{
	Chat = 0,
	Pos
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


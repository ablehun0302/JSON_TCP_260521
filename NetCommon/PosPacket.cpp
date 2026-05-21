#include "pch.h"
#include "PosPacket.h"

PosPacket::PosPacket()
{
}

PosPacket::~PosPacket()
{
}

PacketType PosPacket::GetType()
{
    return PacketType::Pos;
}

void PosPacket::Parse(std::string InString)
{
    JSONDocument.Parse(InString.c_str());

    UserIcon = JSONDocument["UserIcon"].GetString();
    PosX = JSONDocument["PosX"].GetInt();
    PosY = JSONDocument["PosY"].GetInt();
}

std::string PosPacket::ToString()
{
    JSONDocument.SetObject();
    JSONDocument.AddMember("UserIcon", UserIcon, JSONDocument.GetAllocator());
    JSONDocument.AddMember("PosX", PosX, JSONDocument.GetAllocator());
    JSONDocument.AddMember("PosY", PosY, JSONDocument.GetAllocator());

    rapidjson::StringBuffer Buffer;
    rapidjson::Writer<rapidjson::StringBuffer> Writer(Buffer);
    JSONDocument.Accept(Writer);

    return Buffer.GetString();
}

int PosPacket::Length()
{
    return (int)ToString().length();
}

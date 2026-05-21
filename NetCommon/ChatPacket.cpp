#include "pch.h"
#include "ChatPacket.h"
#include <sstream>

//{
//    "UserID" : "junios",
//    "Message" : "Hello World",
//    "Message2" : "Hello World",
//    "Gold" : 1000
//}

ChatPacket::ChatPacket()
{
}

ChatPacket::~ChatPacket()
{
}

PacketType ChatPacket::GetType()
{
    return PacketType::Chat;
}

void ChatPacket::Parse(std::string InString)
{
    JSONDocument.Parse(InString.c_str());

    UserID = JSONDocument["UserID"].GetString();
    Icon = JSONDocument["Icon"].GetString();
    MoveCode = JSONDocument["MoveCode"].GetString();
}

std::string ChatPacket::ToString()
{
    //JSONDocument를 문자열 변환 요청
    JSONDocument.SetObject();
    JSONDocument.AddMember("UserID", UserID, JSONDocument.GetAllocator());
    JSONDocument.AddMember("Icon", Icon, JSONDocument.GetAllocator());
    JSONDocument.AddMember("MoveCode", MoveCode, JSONDocument.GetAllocator());

    rapidjson::StringBuffer Buffer;
    rapidjson::Writer<rapidjson::StringBuffer> Writer(Buffer);
    JSONDocument.Accept(Writer);

    return Buffer.GetString();
}

int ChatPacket::Length()
{
    return (int)ToString().length();
}

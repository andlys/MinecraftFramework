#include "MinecraftBot.h"
#include "Packet.h"
#include "PacketsList.h"
#include <iostream>

using namespace std;
using namespace Packets;
using namespace ClientPackets;

MinecraftBot::MinecraftBot(const char* addr, const char* port):
	_socket(addr, port), _bufferedIO(_socket)
{
	if(_socket.isConnected())
	{
		cout << "Connection succes" << endl;
	}
	else
	{
		cout << "Can't connect to host " << addr << ':' << port << endl;
		//throw MinecraftBotExceptions::NoHostConnection;
	}
}

MinecraftBot::~MinecraftBot()
{
	_socket.close();
}

long MinecraftBot::readPacketID()
{
	MinecraftTypes::VarInt res(0);
	_bufferedIO.buffer().offset() = 0; //to read first bytes
	res.read(_bufferedIO.buffer());
	return res._val;
}

void MinecraftBot::handshake()
{
	//TODO implement method
	long protoVer = 5;
	char* host = "platinium.ddns.net";
	MinecraftTypes::UShort port = 25565; //default port
	long nextState = 2; // get server status
	// 1 - status
	// 2 - login
	HandshakePacket handshake(protoVer, host, port, nextState);
	_bufferedIO.sendData(handshake.dump());
}

void MinecraftBot::login()
{
	handshake();
	LoginStartPacket pack("CPP_Bot");
	_bufferedIO.sendData(pack.dump());
}

int MinecraftBot::startHandling()
{
	login();
	while(true)
	{
		_bufferedIO.readData();
		if(_bufferedIO.buffer().size() == 0)
		{
			std::cout << "Cant read data to handling. Connection broken." << std::endl;
			break;
		}
		long packetID = readPacketID();
		std::cout << "Received packet 0x" << hex << packetID << dec;
		Packets::Packet* packet = Packets::Packet::getServerPacket(packetID);
		if(packet != NULL)
		{
			packet->load(_bufferedIO.buffer());
			std::cout << "\tloadeded ";
			packet->handle(_bufferedIO);
			std::cout << "\thandled" << std::endl;
			delete packet; // delete if only packet was created
		}
		else
		{
			std::cout << "\tnot found" << std::endl;
		}
	}
	return 0;
}

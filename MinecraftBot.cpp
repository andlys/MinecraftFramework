#include "MinecraftBot.h"
#include "Packet.h"
#include "PacketsList.h"
#include <iostream>
#include <string>

using namespace std;
using namespace Packets;
using namespace ClientPackets;

MinecraftBot::MinecraftBot(const char* addr, const char* port, long protocolVersion) :
	_socket(addr, port),
	_bufferedIO(_socket),
	_protocolVersion(protocolVersion),
	_host(addr),
	_port(static_cast<short>(std::stoi(port))),
	_isLoggedIn(false)
{
	if(_socket.isConnected())
	{
		cout << "Connection succes" << endl;
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

bool MinecraftBot::handshake()
{
	long nextState = 2; // get server status
	// 1 - status
	// 2 - login
	if(_socket.isConnected())
	{
		HandshakePacket handshake(_protocolVersion, _host.cstring(), _port, nextState);
		_bufferedIO.sendData(handshake.dump());
	}
	return _socket.isConnected();
}

void MinecraftBot::login(const char* name)
{
	if(handshake())
	{
		LoginStartPacket pack(name);
		_bufferedIO.sendData(pack.dump());
		if(_socket.isConnected())
		{
			_isLoggedIn = true;
		}
	}
}

int MinecraftBot::startHandling()
{
	while(_isLoggedIn && _socket.isConnected())
	{
		_bufferedIO.readData();
		/*if(_bufferedIO.buffer().size() == 0)
		{
			std::cout << "Cant read data to handling. Connection broken." << std::endl;
			break;
		}*/
		long packetID = readPacketID();
		//std::cout << "Received packet 0x" << hex << packetID << dec;
		Packets::Packet* packet = Packets::Packet::getServerPacket(packetID);
		if(packet != NULL)
		{
			packet->load(_bufferedIO.buffer());
			//std::cout << "\tloadeded ";
			packet->handle(_bufferedIO);
			std::cout << packetID << "\thandled" << std::endl;
			delete packet; // delete if only packet was created
		}
		else
		{
			//std::cout << "\tnot found" << std::endl;
		}
	}
	return 0;
}

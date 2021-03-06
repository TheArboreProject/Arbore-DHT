/*
 * Copyright(C) 2008 Laurent Defert, Romain Bignon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <exception>
#include <fcntl.h>
#include <list>
#include <vector>

#include <util/pf_thread.h>
#include <chimera/chimera.h>

#include "pf_addr.h"
#include "packet.h"

class MyConfig;
class ResendPacketJob;

class Network : public Thread, protected Mutex
{
public:
	static const double RETRANSMIT_INTERVAL = 1.0; /**< Seconds before we try to retransmit a packet */
	static const unsigned int MAX_RETRY = 3;       /**< Maximum tries before abording resend a packet */
	static const size_t PACKET_MAX_SIZE = 1024;    /**< Maximum size for packets */

	/* Exceptions */
	class CantOpenSock : public std::exception {};
	class CantListen : public std::exception
	{
		public:
			int port;
			CantListen(int _port) : port(_port) {}
			~CantListen() throw() {};
	};
	class CantConnectTo : public std::exception
	{
		public:
			int err;
			pf_addr addr;
			CantConnectTo(int _errno, const pf_addr _addr) : err(_errno), addr(_addr) {}
			~CantConnectTo() throw() {};
	};

private:

	typedef std::set<int> SockSet;
	SockSet socks;    /**< contains all socks listened */
	fd_set socks_fd_set;
	int highsock;     /** higher socket opened, used by select POSIX function */

	std::vector<ResendPacketJob*> resend_list;
	uint32_t seqend;
	Chimera *chimera_;

	void CloseAll();
	void Loop();
	void OnStop();

public:

	/** Constructor of network.
	 * @param chimera an instance of the Chimera routing layer.
	 */
	Network(Chimera *chimera);
	~Network();

	/** Listen an UDP port.
	 *
	 * @param chimera the Chimera routing layer
	 * @param port  the listened port
	 * @param bind_addr  the listened address
	 * @return  the file descriptor
	 */
	int Listen(uint16_t port, const char* bind_addr);

	/* Read configuration and start listener, and connect to other servers */
	virtual void StartNetwork(MyConfig* conf);

	/** Send a packet.
	 * @param sock the socket
	 * @param host the Host which will receive the message
	 * @param pckt the Packet to send
	 * @return true if success, false otherwise
	 */
	bool Send(int sock, Host host, Packet pckt);
};

#endif /* NETWORK_H */

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
 * $Id$
 */

#ifndef PEER_H
#define PEER_H

#include "pf_types.h"
#include "packet.h"
#include <queue>

class FileEntry;
class Peer;

typedef std::vector<Peer*> PeerList;

class Peer
{
	int fd;
	pf_addr addr;
	int ts_diff; // diff between our timestamp and its timestamp */
	Packet* incoming; // packet we are receiving
	std::queue<Packet> send_queue; // packets we are sending (with flush)

	/* Network is representer for me like this:
	 *  me
	 *  |- direct downpeer1
	 *  |  |- downpeer2
	 *  |  |  `- downpear3
	 *  |  `- downpeer4
	 *  `- direct downpeer5
	 *
	 *  If this Peer is "downpeer2", uplink points to "direct downpeer1",
	 *  and downlinks contain "downpeer3".
	 */
	Peer* uplink;                            /**< Peer where this peer is connected */
	std::vector<Peer*> downlinks;            /**< Peer connected to this one */

	unsigned int flags;

	// Sending functions
	void Send_net_peer_list(PeerList list);
	// Receiving functions
	void Handle_net_hello(struct Packet* pckt);
	void Handle_net_mkfile(struct Packet* pckt);
	void Handle_net_rmfile(struct Packet* pckt);
	void Handle_net_peer_connection(struct Packet* pckt);
	void Handle_net_start_merge(struct Packet* pckt);
	void Handle_net_end_of_merge(struct Packet* pckt);
	void Handle_net_end_of_merge_ack(struct Packet* pckt);
public:

	/* Exceptions */
	class MustDisconnect : public std::exception {};

	enum
	{
		SERVER     = 1 << 0,    /**< This peer is a server */
		HIGHLINK   = 1 << 1,    /**< This peer is a highlink */
		MERGING    = 1 << 2,    /**< We are merging with this peer (between HELLO and END_OF_MERGE) */
		ANONYMOUS  = 1 << 3,    /**< We don't know this peer (between connection and HELLO) */
	};

	/* Constructors */
	Peer(int _fd, pf_addr addr, Peer* = 0);
	~Peer();

	id_t GetID() const { return addr.id; }
	int GetFd() const { return fd; }
	pf_addr GetAddr() const { return addr; }

	time_t Timestamp(time_t ts) { return ts_diff + ts; }

	bool IsHighLink() const { return flags & HIGHLINK; }
	bool IsLowLink() const { return !(flags & HIGHLINK); }
	void SetHighLink(bool l = true) { l ? SetFlag(HIGHLINK) : DelFlag(HIGHLINK); }

	bool IsServer() const { return (flags & SERVER); }
	bool IsClient() const { return !(flags & SERVER); }

	bool IsDirectLink() const { return !uplink; }

	void SetFlag(unsigned int f) { flags |= f; }
	void DelFlag(unsigned int f) { flags &= ~f; }
	bool HasFlag(unsigned int f) { return flags & f; }

	void HandleMsg(struct Packet* pckt);

	void Flush();
	void SendMsg(const PacketBase& pckt);
	void SendHello();
	void Receive();
};

#endif

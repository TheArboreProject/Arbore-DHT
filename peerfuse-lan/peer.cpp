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

#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include "cache.h"
#include "packet.h"
#include "peer.h"
#include "network.h"
#include "net_proto.h"
#include "pf_types.h"
#include "pflan.h"
#include "log.h"
#include "job_other_connect.h"
#include "session_config.h"
#include "tools.h"

Peer::Peer(int _fd, pf_addr _addr, Connection* _conn)
			: fd(_fd),
			addr(_addr),
			conn(_conn),
			ts_diff(0),
			incoming(NULL),
			flags(0)
{
}

Peer::~Peer()
{
	if(fd)
		close(fd);
	delete incoming;
	delete conn;
}

void Peer::Flush()
{
	while(!send_queue.empty())
	{
		send_queue.front().Send(fd);
		send_queue.pop();
	}
}

/****************************
 *  Send
 */

void Peer::SendMsg(const PacketBase& pckt)
{
	send_queue.push(pckt);
	net.HavePacketToSend(this);
}

void Peer::SendHello()
{
	// Make the message
	Packet pckt(NET_HELLO);
						  // Time / now
	pckt.SetArg(NET_HELLO_NOW, (uint32_t)time(NULL));
	pckt.SetArg(NET_HELLO_PORT, (uint32_t)net.GetListeningPort());
	pckt.SetArg(NET_HELLO_VERSION, std::string(PEERFUSE_VERSION));
	pckt.SetArg(NET_HELLO_MY_ID, net.GetMyID());
	SendMsg(pckt);
}

/*******************************
 * Handers
 */

void Peer::Handle_net_hello(struct Packet* pckt)
{
	if(pckt->GetArg<std::string>(NET_HELLO_VERSION) != std::string(PEERFUSE_VERSION))
	{
		log[W_WARNING] << "Versions are different !";
		throw MustDisconnect();
	}

	ts_diff = time(NULL) - pckt->GetArg<uint32_t>(NET_HELLO_NOW);
	addr.port = pckt->GetArg<uint32_t>(NET_HELLO_PORT);

	if(IsServer())
	{
		// Initiate the merge if it's the first peer we connect to
		if(net.GetPeerList().size() == 1)
		{
			net.SetMerging(true);
			SetFlag(MERGING);
			SendMsg(Packet(NET_START_MERGE));
		}
	}
	else
		SendHello();

	if(pckt->GetArg<uint32_t>(NET_HELLO_MY_ID) == 0)
	{
		// The peer don't have an ID, give him one
		// TODO: check it's not already used
		SendMsg(Packet(NET_YOUR_ID).SetArg(NET_YOUR_ID_ID, net.CreateID()));
	}
	else
		addr.id = pckt->GetArg<uint32_t>(NET_HELLO_MY_ID);
}

void Peer::Handle_net_your_id(struct Packet* pckt)
{
	net.SetMyID(pckt->GetArg<uint32_t>(NET_YOUR_ID_ID));
	session_cfg.Set("my_id", net.GetMyID());
}

void Peer::Handle_net_start_merge(struct Packet* pckt)
{
	SetFlag(MERGING);

	net.scheduler.Queue(new JobOtherConnect(this));
}

void Peer::Handle_net_peer_connection(struct Packet* msg)
{
	pf_addr new_peer = msg->GetArg<pf_addr>(NET_PEER_CONNECTION_ADDRESS);

	try
	{
		net.Connect(new_peer);
	}
	catch(Network::CantConnectTo &e)
	{
		// acknowledge the peer this peer can't be contacted
		SendMsg(Packet(NET_PEER_CONNECTION_RST).SetArg(NET_PEER_CONNECTION_RST_ADDRESS, new_peer));
		return;
	}

	// acknowledge the peer the connection is established
	SendMsg(Packet(NET_PEER_CONNECTION_ACK).SetArg(NET_PEER_CONNECTION_ACK_ADDRESS, new_peer));
}

void Peer::Handle_net_peer_connection_ack(struct Packet* msg)
{
	pf_addr new_peer = msg->GetArg<pf_addr>(NET_PEER_CONNECTION_ACK_ADDRESS);
	std::list<Job*>& jobs = net.scheduler.GetQueue();

	for(std::list<Job*>::iterator it = jobs.begin();
		it != jobs.end();
		++it)
	{
		if((*it)->GetType() == JOB_OTHER_CONNECT)
		{
			JobOtherConnect* j = static_cast<JobOtherConnect*>(*it);
			if(j->IsConnectingTo(new_peer))
				j->PeerConnected(this);
		}
	}
}

void Peer::Handle_net_peer_connection_rst(struct Packet* msg)
{
	pf_addr new_peer = msg->GetArg<pf_addr>(NET_PEER_CONNECTION_RST_ADDRESS);
	std::list<Job*> jobs = net.scheduler.GetQueue();

	for(std::list<Job*>::iterator it = jobs.begin();
		it != jobs.end();
		++it)
	{
		if((*it)->GetType() == JOB_OTHER_CONNECT)
		{
			JobOtherConnect* j = static_cast<JobOtherConnect*>(*it);
			if(j->IsConnectingTo(new_peer))
				net.scheduler.Cancel(j);
		}
	}

	// notifiy the peer he can't be contacted by *this
	PeerList peers = net.GetPeerList();

	for(PeerList::iterator it = peers.begin(); it != peers.end(); ++it)
		if((*it)->GetAddr() == new_peer)
	{
		(*it)->SendMsg(Packet(NET_PEER_CONNECTION_REJECTED).SetArg(NET_PEER_CONNECTION_REJECTED_ADDRESS, GetAddr()));
		break;
	}
}

void Peer::Handle_net_peer_connection_rejected(struct Packet* msg)
{
	pf_addr peer = msg->GetArg<pf_addr>(NET_PEER_CONNECTION_REJECTED_ADDRESS);

	log[W_ERR] << "Host " << peer << " can't connect.";
	throw PeerCantConnect(peer);
}

void Peer::Handle_net_peer_all_connected(struct Packet* msg)
{
	Packet pckt(NET_GET_STRUCT_DIFF);
	uint32_t last_v = 0;
	session_cfg.Get("last_view", last_v);
	pckt.SetArg(NET_GET_STRUCT_DIFF_LAST_CONNECTION, last_v);
	SendMsg(pckt);
}

void Peer::Handle_net_get_struct_diff(struct Packet* pckt)
{
	time_t last_view = Timestamp(pckt->GetArg<uint32_t>(NET_GET_STRUCT_DIFF_LAST_CONNECTION));

	cache.SendChanges(this, last_view);
}

void Peer::Handle_net_end_of_diff(struct Packet* pckt)
{
	/* TODO: send my own modifications */
	SendMsg(Packet(NET_END_OF_MERGE));
}

void Peer::Handle_net_mkfile(struct Packet* msg)
{
	cache.Lock();
	std::string filename;
	try
	{
		filename = msg->GetArg<std::string>(NET_MKFILE_PATH);
		mode_t mode = msg->GetArg<uint32_t>(NET_MKFILE_MODE);

		FileEntry* leaf = cache.MkFile(filename, mode);

		leaf->stat.uid = msg->GetArg<uint32_t>(NET_MKFILE_UID);
		leaf->stat.gid = msg->GetArg<uint32_t>(NET_MKFILE_GID);
		leaf->stat.size = msg->GetArg<uint64_t>(NET_MKFILE_SIZE);
		leaf->stat.atime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_ACCESS_TIME));
		leaf->stat.mtime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_MODIF_TIME));
		leaf->stat.ctime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_CREATE_TIME));
	}
	catch(Cache::NoSuchFileOrDir &e)
	{
		log[W_DESYNCH] << "Unable to create " << filename << ": No such file or directory";
		/* XXX: Desync DO SOMETHING */
	}
	catch(Cache::FileAlreadyExists &e)
	{
		log[W_DESYNCH] << "Unable to create " << filename << ": File already exists";
		/* XXX: DO SOMETHING */
	}
	cache.Unlock();
}

void Peer::Handle_net_rmfile(struct Packet* msg)
{
	cache.Lock();
	try
	{
		cache.RmFile(msg->GetArg<std::string>(NET_RMFILE_PATH));
	}
	catch(Cache::NoSuchFileOrDir &e)
	{
		log[W_DESYNCH] << "Unable to remove " << msg->GetArg<std::string>(NET_RMFILE_PATH) << ": No such file or directory";
		/* TODO: Desynch, DO SOMETHING */
	}
	catch(Cache::DirNotEmpty &e)
	{
		log[W_DESYNCH] << "Unable to remove " << msg->GetArg<std::string>(NET_RMFILE_PATH) << ": Dir not empty";
		/* TODO: Desynch, DO SOMETHING */
	}
	cache.Unlock();
}

void Peer::Handle_net_end_of_merge(struct Packet* msg)
{
	net.SetMerging(false);
	DelFlag(MERGING);
	if(IsClient())
	{
		SendMsg(Packet(NET_END_OF_MERGE_ACK));
		session_cfg.Set("last_view", time(NULL));
	}
}

void Peer::Handle_net_end_of_merge_ack(struct Packet* msg)
{
	DelFlag(MERGING);
}

void Peer::HandleMsg(Packet* pckt)
{
	void (Peer::*handler[NET_NB_MESSAGES]) (Packet*) =
	{
		NULL,
		&Peer::Handle_net_hello,
		&Peer::Handle_net_your_id,
		&Peer::Handle_net_start_merge,
		&Peer::Handle_net_get_struct_diff,
		&Peer::Handle_net_mkfile,
		&Peer::Handle_net_rmfile,
		&Peer::Handle_net_peer_connection,
		&Peer::Handle_net_peer_connection_ack,
		&Peer::Handle_net_peer_connection_rst,
		&Peer::Handle_net_peer_connection_rejected,
		&Peer::Handle_net_peer_all_connected,
		&Peer::Handle_net_end_of_diff,
		&Peer::Handle_net_end_of_merge,
		&Peer::Handle_net_end_of_merge_ack,
	};

	/* Note that we can safely cast pckt->type to unsigned after check pkct->type > 0 */
	if(pckt->GetType() <= 0)
		throw Packet::Malformated();
	if((size_t)pckt->GetType() >= (sizeof handler/ sizeof *handler))
		throw Packet::Malformated();
	if((size_t)pckt->GetType() >= NET_NB_MESSAGES)
		throw Packet::Malformated();

	(this->*handler[pckt->GetType()])(pckt);
}

void Peer::Receive()
{
	/* If there was already an incoming packet, we can receive its content */
	if(incoming)
		incoming->ReceiveContent(fd);
	else
	{
		/* This is a new packet, we only receive the header */
		char header[ Packet::GetHeaderSize() ];

		if(recv(fd, &header, Packet::GetHeaderSize(), 0) <= 0)
			throw Packet::RecvError();

		incoming = new Packet(header);

		log[W_PARSE] << "Received a message header: type=" << incoming->GetType() << ", " <<
			" size=" << incoming->GetDataSize();

		/* If there some data in packet, we wait for the rest on the next Receive() call.
		 * In other case, it is because packet only contains headers and we can parse it.
		 */
		if(incoming->GetDataSize() > 0)
			return;
	}

	/* We use the Deleter class because we don't know how we will
	 * exit this function. With it, we are *sure* than Packet instance
	 * will be free'd.
	 */
	Deleter<Packet> packet(incoming);
	incoming = NULL;

	HandleMsg(*packet);
}

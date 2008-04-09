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
#include "packet.h"
#include "peer.h"
#include "environment.h"
#include "net_proto.h"
#include "pf_types.h"
#include "pflan.h"
#include "log.h"
#include "job_other_connect.h"
#include "job_new_conn_req.h"
#include "job_flush_peer.h"
#include "job_mkfile.h"
#include "job_rmfile.h"
#include "job_send_changes.h"
#include "session_config.h"
#include "tools.h"
#include "peers_list.h"
#include "scheduler_queue.h"

Peer::Peer(pf_addr _addr, Connection* _conn, unsigned int _flags) :
			PeerBase(_addr, _conn, _flags)
{
	if(IsServer())
		SendHello();
}

Peer::~Peer()
{
}

/****************************
 *  Send
 */

void Peer::SendMsg(const Packet& pckt)
{
	log[W_PARSE] << "-> (" << GetFd() << "/" << GetID() << ") " << pckt.GetPacketInfo();
	send_queue.push(pckt);
	scheduler_queue.Queue(new JobFlushPeer(GetFd()));
}

void Peer::SendHello()
{
	// Make the message
	Packet pckt(NET_HELLO);
						  // Time / now
	pckt.SetArg(NET_HELLO_NOW, (uint32_t)time(NULL));
	pckt.SetArg(NET_HELLO_PORT, (uint32_t)environment.listening_port.Get());
	pckt.SetArg(NET_HELLO_VERSION, std::string(PEERFUSE_VERSION));
	pckt.SetArg(NET_HELLO_MY_ID, environment.my_id.Get());
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
	addr.port = (uint16_t) pckt->GetArg<uint32_t>(NET_HELLO_PORT);

	if(IsClient())
		SendHello();

	if(pckt->GetArg<uint32_t>(NET_HELLO_MY_ID) == 0)
	{
		// The peer don't have an ID, give him one
		// TODO: check it's not already used
		Packet p(NET_YOUR_ID);
		addr.id = peers_list.CreateID();
		p.SetArg(NET_YOUR_ID_ID, addr.id);
		SendMsg(p);
	}
	else
		addr.id = pckt->GetArg<uint32_t>(NET_HELLO_MY_ID);

	if(IsServer())
	{
		// Initiate the merge if it's the first peer we connect to
		if(peers_list.Size() == 1)
		{
			environment.merging.Set(true);
			SetFlag(MERGING);
			SendMsg(Packet(NET_START_MERGE));
		}
	}
}

void Peer::Handle_net_your_id(struct Packet* pckt)
{
	if(environment.my_id.Get() != 0)
		throw MustDisconnect();
	environment.my_id.Set(pckt->GetArg<uint32_t>(NET_YOUR_ID_ID));
	session_cfg.Set("my_id", environment.my_id.Get());
	log[W_INFO] << "My ID now is " << environment.my_id.Get();
}

void Peer::Handle_net_start_merge(struct Packet* pckt)
{
	SetFlag(MERGING);

	scheduler_queue.Queue(new JobOtherConnect(GetID()));
}

void Peer::Handle_net_peer_connection(struct Packet* msg)
{
	pf_addr new_peer = msg->GetArg<pf_addr>(NET_PEER_CONNECTION_ADDRESS);

	scheduler_queue.Queue(new JobNewConnReq(new_peer, addr.id));
}

void Peer::Handle_net_peer_connection_ack(struct Packet* msg)
{
	pf_addr new_peer = msg->GetArg<pf_addr>(NET_PEER_CONNECTION_ACK_ADDRESS);

	scheduler_queue.Lock();
	for(SchedulerQueue::iterator it = scheduler_queue.begin();
		it != scheduler_queue.end();
		++it)
	{
		if((*it)->GetType() == JOB_OTHER_CONNECT)
		{
			JobOtherConnect* j = static_cast<JobOtherConnect*>(*it);
			if(j->IsConnectingTo(new_peer))
				j->PeerConnected(GetID());
		}
	}
	scheduler_queue.Unlock();
}

void Peer::Handle_net_peer_connection_rst(struct Packet* msg)
{
	pf_addr new_peer = msg->GetArg<pf_addr>(NET_PEER_CONNECTION_RST_ADDRESS);

	scheduler_queue.Lock();
	/* we will erase some data from scheduler's queue, so
	 * do a copy of it here. */
	SchedulerQueue jobs = scheduler_queue;
	for(SchedulerQueue::iterator it = jobs.begin();
		it != jobs.end();
		++it)
	{
		if((*it)->GetType() == JOB_OTHER_CONNECT)
		{
			JobOtherConnect* j = static_cast<JobOtherConnect*>(*it);
			if(j->IsConnectingTo(new_peer))
				scheduler_queue.Cancel(j);
		}
	}
	scheduler_queue.Unlock();

	// notifiy the peer he can't be contacted by *this
	BlockLockMutex lock(&peers_list);
	for(PeersList::iterator it = peers_list.begin(); it != peers_list.end(); ++it)
		if((*it)->GetAddr() == new_peer)
	{
		Packet p(NET_PEER_CONNECTION_REJECTED);
		p.SetArg(NET_PEER_CONNECTION_REJECTED_ADDRESS, GetAddr());
		(*it)->SendMsg(p);
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

	scheduler_queue.Queue(new JobSendChanges(GetID(), last_view));
}

void Peer::Handle_net_end_of_diff(struct Packet* pckt)
{
	/* TODO: send my own modifications */
	SendMsg(Packet(NET_END_OF_MERGE));
}

void Peer::Handle_net_mkfile(struct Packet* msg)
{
	std::string filename;
	filename = msg->GetArg<std::string>(NET_MKFILE_PATH);

	pf_stat stat;
	stat.mode = msg->GetArg<uint32_t>(NET_MKFILE_MODE);
	stat.uid = msg->GetArg<uint32_t>(NET_MKFILE_UID);
	stat.gid = msg->GetArg<uint32_t>(NET_MKFILE_GID);
	stat.size = msg->GetArg<uint64_t>(NET_MKFILE_SIZE);
	stat.atime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_ACCESS_TIME));
	stat.mtime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_MODIF_TIME));
	stat.meta_mtime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_META_MODIF_TIME));
	stat.ctime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_CREATE_TIME));

	scheduler_queue.Queue(new JobMkFile(filename, stat, GetID()));
}

void Peer::Handle_net_rmfile(struct Packet* msg)
{
	std::string filename = msg->GetArg<std::string>(NET_RMFILE_PATH);
	scheduler_queue.Queue(new JobRmFile(filename, GetID()));
}

void Peer::Handle_net_end_of_merge(struct Packet* msg)
{
	environment.merging.Set(false);
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

bool Peer::Receive()
{
	if(!PeerBase::ReceivePacket())
		return false;

	/* We use the Deleter class because we don't know how we will
	 *  * exit this function. With it, we are *sure* than Packet instance
	 *   * will be free'd.
	 *    */
	Deleter<Packet> packet(incoming);
	incoming = NULL;

	HandleMsg(*packet);

	return false;
}

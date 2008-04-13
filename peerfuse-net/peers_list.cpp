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

#include "peers_list.h"
#include "peers_list_base.h"
#include "environment.h"

PeersList peers_list;

void PeersList::GivePacketTo(pf_id id, Packet* packet) const
{
	BlockLockMutex lock(&peers_list);
	const_iterator it;
	/* Packets are not given to anonymous Peers */
	for(it = begin(); it != end() && ((*it)->GetID() != id || (*it)->IsAnonymous()); ++it)
		;

	if(it != end())
		(*it)->HandleMsg(packet);
	else
		log[W_WARNING] << "Received a packet from unknown peer";
}

void PeersList::SendPeerList(Peer* to) const
{
	/* This is an alias, to disallow usage of _send_peer_list
	 * with second argument out of PeersList object.
	 */
	_send_peer_list(to, NULL);
}

void PeersList::_send_peer_list(Peer* to, Peer* from) const
{
	BlockLockMutex lock(&peers_list);
	StaticPeersList peers = from ? GetDownLinks(from) : GetDirectHighLinks();
	for(StaticPeersList::iterator it = peers.begin(); it != peers.end(); ++it)
	{
		/* Do not send information about himself! */
		if(*it == to)
			continue;

		pf_id id = (*it)->GetUpLink() ? (*it)->GetUpLink() : environment.my_id.Get();

		/* It broadcasts. */
		Packet pckt(NET_PEER_CONNECTION, id, 0);
		pckt.SetArg(NET_PEER_CONNECTION_ADDRESS, (*it)->GetAddr());
		/* TODO: put certificate here! */
		pckt.SetArg(NET_PEER_CONNECTION_CERTIFICATE, std::string("TODO: put certificate here"));

		to->SendMsg(pckt);

		_send_peer_list(to, *it);
	}
}

void PeersList::RemoveDownLinks(Peer* p)
{
	BlockLockMutex lock(this);

	/* Become highlink */
	std::vector<Peer*> down_links = GetDownLinks(p);
	for(std::vector<Peer*>::iterator it = down_links.begin();
		it != down_links.end();
		++it)
	{
		//AddDisconnected((*it)->GetAddr());
	}
}

StaticPeersList PeersList::GetDirectHighLinks() const
{
	BlockLockMutex lock(&peers_list);
	StaticPeersList list;
	const_iterator it;
	for(it = begin(); it != end(); ++it)
		if((*it)->IsHighLink() && !(*it)->IsAnonymous())
			list.push_back(*it);

	return list;
}

StaticPeersList PeersList::GetDownLinks(Peer* p) const
{
	BlockLockMutex lock(&peers_list);
	StaticPeersList list;
	std::vector<pf_id> downlinks = p->GetDownLinks();
	for(std::vector<pf_id>::iterator it = downlinks.begin();
		it != downlinks.end();
		++it)
	{
		Peer* peer = PeerFromID(*it);
		assert(peer);
		list.push_back(peer);
	}
	return list;
}

Peer* PeersList::RemoveFromID(pf_id id)
{
	BlockLockMutex lock(this);
	iterator it;
	for(it = begin(); it != end() && (*it)->GetID() != id; ++it)
		;

	if(it == end())
		return NULL;

	Peer* peer = *it;
	erase(it);

	if(peer->IsConnection())
	{
		PeerMap::iterator p = fd2peer.find(peer->GetFd());
		if(p != fd2peer.end())
			fd2peer.erase(p);
	}

	return peer;
}

void PeersList::EraseFromID(pf_id id)
{
	BlockLockMutex lock(this);
	Peer* p = RemoveFromID(id);
	delete p;
}

void PeersList::Broadcast(Packet pckt, const Peer* but_one) const
{
	BlockLockMutex lock(&peers_list);
	pckt.SetDstID(0);
	PeersList::iterator it;
	for(it = peers_list.begin(); it != peers_list.end(); ++it)
		if(!(*it)->IsAnonymous() &&
		(*it)->IsHighLink() &&
		(*it) != but_one)
			(*it)->SendMsg(pckt);
}

bool PeersList::IsIDOnNetwork(pf_id id)
{
	return PeerFromID(id);
}

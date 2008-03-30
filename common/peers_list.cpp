#include "peers_list.h"
#include "peer.h"
#include "mutex.h"
#include "log.h"

PeersList peers_list;

PeersList::PeersList() : Mutex(RECURSIVE_MUTEX),
			my_id(0)
{
}

PeersList::~PeersList()
{
	for(iterator it = begin(); it != end(); ++it)
		delete *it;
}

Peer* PeersList::PeerFromFD(int fd)
{
	PeerMap::iterator it = fd2peer.find(fd);
	if(it == fd2peer.end())
		return NULL;
	return it->second;
}

Peer* PeersList::PeerFromID(pf_id id)
{
	iterator it;
	for(it = begin(); it != end() && (*it)->GetID() != id; ++it)
		;

	return (it != end() ? *it : NULL);
}


// Public methods
void PeersList::Add(Peer* p)
{
	BlockLockMutex lock(this);
	push_back(p);
	fd2peer[p->GetFd()] = p;
}

Peer* PeersList::Remove(int fd)
{
	BlockLockMutex lock(this);
	Peer* peer = NULL;
	if(fd2peer.find(fd) != fd2peer.end())
	{
		PeerMap::iterator p = fd2peer.find(fd);
		log[W_CONNEC] << "<- Removing a peer: " << fd << " (" << p->second->GetID() << ")";

		for(iterator it = begin(); it != end();)
			if(*it == p->second)
				it = erase(it);
		else
			++it;

		peer = p->second;
		fd2peer.erase(p);
	}
	return peer;
}

void PeersList::Erase(int fd)
{
	BlockLockMutex lock(this);
	Peer* p = Remove(fd);
	delete p;
}

void PeersList::PeerFlush(int fd)
{
	BlockLockMutex lock(this);
	Peer* p = PeerFromFD(fd);
	if(p)
		p->Flush();
}

bool PeersList::PeerReceive(int fd)
{
	BlockLockMutex lock(this);
	bool res = 0;
	Peer* p = PeerFromFD(fd);
	if(p)
		res = p->Receive();
	return res;
}

void PeersList::CloseAll()
{
	BlockLockMutex lock(this);
	for(iterator it = begin(); it != end(); ++it)
		delete *it;
	clear();
	fd2peer.clear();
}

pf_id PeersList::CreateID()
{
	BlockLockMutex lock(this);
	// TODO: optimize me
	pf_id new_id = 0;
	while(!new_id)
	{
		new_id = rand();
		if(new_id == GetMyID())
		{
			new_id = 0;
			continue;
		}
		for(PeerMap::iterator peer = fd2peer.begin();
			peer != fd2peer.end();
			++peer)
		{
			if(new_id == peer->second->GetID())
			{
				new_id = 0;
				break;
			}
		}
	}

	return new_id;
}
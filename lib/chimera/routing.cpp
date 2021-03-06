/*
 * Copyright(C) 2008 Romain Bignon
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
 * This file contains some code from the Chimera's Distributed Hash Table,
 * written by CURRENT Lab, UCSB.
 *
 */

#include "routing.h"

Routing::Routing(Host _me)
	: me(_me),
	routingTable(_me),
	leafset(_me)
{
}

void Routing::KeyUpdate(Host me)
{
	BlockLockMutex lock(this);
	this->leafset.KeyUpdate(me);
	this->routingTable.KeyUpdate(me);
}

bool Routing::add(const Host& host)
{
	BlockLockMutex lock(this);
	bool added = this->leafset.add(host);
	if (!added)
		added = this->routingTable.add(host);
	return added;
}

bool Routing::remove(const Host& host)
{
	BlockLockMutex lock(this);
	bool removed = this->leafset.remove(host);
	if (!removed)
		removed = this->routingTable.remove(host);
	return removed;
}

Host Routing::routeLookup(const Key& key) const
{
	BlockLockMutex lock(this);
	bool b;
	pf_log[W_ROUTING] << "Look if it's for me";
	if(this->me.GetKey() == key)
		return this->me;

	pf_log[W_ROUTING] << "Lookup in the leafset table";
	Host leafsetBest = this->leafset.routeLookup(key , &b);
	if(b)
	{
		return leafsetBest;
	}
	pf_log[W_ROUTING] << "..failed.. Lookup in the routing table";
	Host routingTableBest = this->routingTable.routeLookup(key , &b);
	if(b)
	{
		return routingTableBest;
	}
	pf_log[W_ROUTING] << "..failed.. do some incantations..";
	Key distLB = leafsetBest.GetKey().distance(key);
	Key distRB = routingTableBest.GetKey().distance(key);
	if(distLB < distRB)
	{
		return leafsetBest;
	}
	if(distLB > distRB)
	{
		return routingTableBest;
	}
	//distance is the same
	if(leafsetBest == this->me)
	{
		return leafsetBest;
	}
	return routingTableBest;
}

const Leafset* Routing::GetLeafset() const
{
	return &leafset;
}

const RoutingTable* Routing::GetRoutingTable() const
{
	return &routingTable;
}

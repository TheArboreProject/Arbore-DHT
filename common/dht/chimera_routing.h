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
#ifndef _CHIMERA_ROUTING_H_
#define _CHIMERA_ROUTING_H_

#include "routing_table.h"
#include "leafset.h"

class ChimeraRouting
{
private :
	HostGlobal* hg; /*!< Global peer manager */
	ChimeraHost* me; /*!< Local host descriptor */
	RoutingTable routingTable; /*!< DHT routing table */
	Leafset	leafset; /*!< DHT leafset */

public :
	/*! \brief Constructor
	* Constructor, a new chimera routing system
	* \param hg the global host
	* \param me the local node
	*/
	ChimeraRouting(HostGlobal* hg, ChimeraHost* me);
	void KeyUpdate(ChimeraHost* me);

	/*! \brief Updates the routing information by adding or removing a peer.
	* When a peer joins or leaves the network, this function can add it or remove it from the DHT neighbours that are used for routing.
	* \deprecated use add and remove instead
	* \param host the peer whose status is updated
	* \param joined 1 if the peer joined, 0 if he left
	*/
	void route_update(const ChimeraHost * host, int joined);

	/*! \brief Updates the routing information by adding a peer.
	* When a peer joins the network, this function can add it or remove it from the DHT neighbours that are used for routing.
	* \param entry the peer that can be added
	* \return true if it was added, false if it wasn't
	*/
	bool add(const ChimeraHost* entry);

	/*! \brief Updates the routing information by removing a peer.
	* When a peer leaves the network, this function removes it from the DHT neighbours that are used for routing.
	* \param entry the peer that should be removed
	* \return true if it was added, false if it wasn't part of the neighbours
	*/
	bool remove(const ChimeraHost* entry);

	/*! \brief Finds the next routing destination
	* Finds the best destination for the next step of routing to key. First we look for a final destination in the leafset, then for a prefix match in the routing table, and finally we take the node from the routing table or the leafset which is the closest to the destination.
	* \param key routing destination
	*/
	ChimeraHost* routeLookup(const Key* key) const;
	//ChimeraHost** route_get_table();

	//ChimeraHost** route_get_leafset();

	//void printTable () const;


};

#endif /* _CHIMERA_ROUTING_H_ */
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
 *
 */

#include <util/pf_log.h>
#include <net/network.h>
#include <scheduler/job.h>
#include <util/time.h>
#include <util/pf_types.h>
#include <net/pf_addr.h>

#include "job_new_connection.h"

bool JobNewConnection::Start()
{
	try
	{
		net.Connect(*this);
	}
	catch(Network::CantConnectTo &e)
	{
		pf_log[W_INFO] << "Unable to connect to " << (pf_addr)*this;
		return true;
	}
	return false;
}

bool JobNewConnection::IsMe(const pf_addr& addr)
{
	/* If this peer is the same host/port than me. */
	if(addr.ip[0] == this->ip[0] &&
		addr.ip[1] == this->ip[1] &&
		addr.ip[2] == this->ip[2] &&
		addr.ip[3] == this->ip[3] &&
		addr.port == this->port)
		return true;

	/* If this peer is the same ID than me.
	 * We consider that it tries to connect to
	 * me from an other host/port.
	 */
	if(addr.id == this->id)
		return true;

	return false;
}

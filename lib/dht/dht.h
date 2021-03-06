/*
 * Copyright(C) 2012 Michael Muré <batolettre@gmail.com
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

#ifndef DHT_H
#define DHT_H

#include <chimera/chimera.h>
#include <util/key.h>

#include "data.h"
#include "datakey.h"
#include "datastring.h"
#include "storage.h"
#include <files/arbore.h>

class Arbore;

class DHT
{
public:

	/** Number of host on which data are replicated in each direction.
	 * For instance, a redondancy of 2 means that 5 hosts know the data (including me).
	 */
	static const uint32_t REDONDANCY = 1;

	/* DHT constructor.
	 * @param port the port that we listen on
	 * @param me the key used on the routing layer
	 */
	DHT(Arbore* arbore, uint16_t port, const Key& me = Key::GetRandomKey());
	virtual ~DHT() {}

	/* PUBLISH */

	/** Publish a string on the DHT
	 * @return true if the request was send successfully, false otherwise. */
	void Publish(const Key& id, const std::string string) const;

	/** Publish a key on the DHT
	 * @return true if the request was send successfully, false otherwise. */
	void Publish(const Key& id, const Key& key) const;

	/** Publish a Data on the DHT
	 * @return true if the request was send successfully, false otherwise. */
	void Publish(const Key& id, Data* data) const;

	/* UNPUBLISH */

	/** Unpublish a string on DHT
	 * @return true if the request was send successfully, false otherwise. */
	void Unpublish(const Key& id, const std::string string) const;

	/** Unpublish a key on DHT
	 * @return true if the request was send successfully, false otherwise. */
	void Unpublish(const Key& id, const Key& key) const;

	/** Unpublish a list of keys on DHT
	 * @return true if the request was send successfully, false otherwise. */
	void Unpublish(const Key& id, Data* data) const;

	/* Request a value in the DHT
	 * @return true if a request need to be sent, false otherwise.
	 * If false is returned, it does not mean that the data is available,
	 * because the data may just not exist.
	 */
	bool RequestData(const Key& id) const;

	/** Handle a network message.
	 * @return true if the request was send successfully, false otherwise. */
	void HandleMessage(const Host& sender, const Packet& pckt);

	/** @return the chimera routing layer */
	Chimera* GetChimera() const;

	/** @return the DHT storage object */
	Storage* GetStorage() const;

	/** @return the key used as id on the network */
	const Key& GetMe() const;

	/** @return the Arbore above */
	Arbore* GetArbore() const;

private:
	Arbore *arbore_;
	const Key& me_;
	Chimera *chimera_;
	Storage *storage_;
};

#endif

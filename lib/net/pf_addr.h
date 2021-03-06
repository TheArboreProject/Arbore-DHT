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

#ifndef PF_ADDR_H
#define PF_ADDR_H

#include <netinet/in.h>

#include <util/pf_log.h>
#include <util/pf_types.h>
#include <util/key.h>

const size_t ip_t_len = 4;
typedef uint32_t ip_t[4];

/** This class holds an adress of a host, defined by it's ip adress (ipv4/6),
 * port and key.
 */
class pf_addr
{
public:
	static const size_t size = sizeof(ip_t) +       /* ip */
	                           sizeof(uint16_t) +   /* port */
	                           Key::size;           /* key */
	ip_t ip;
	uint16_t port;
	Key key;

	class CantResolvHostname : public std::exception {};

	pf_addr();
	pf_addr(in_addr_t address_v4, uint16_t port, Key key = Key());
	pf_addr(const char* buf);
	pf_addr(std::string hostname, uint16_t port);
	~pf_addr() {}

	void dump(char* buf) const;

	/** Comparaison between two pf_addr
	 *
	 * @param other this is the other pf_addr which is compared to me
	 * @return true if the two objects are equal.
	 *
	 * \note The key isn't compared if one of the two is NULL. In that case, false is returned.
	 */
	bool operator ==(const pf_addr &other) const;

	/** Comparaison between two pf_addr
	 *
	 * @param other this is the other pf_addr which is compared to me
	 * @return true if my address is before the other's.
	 *
	 * \note The key isn't compared if one of the two is NULL. In that case, false is returned.
	 */

	bool operator<(const pf_addr &other) const;

	/** @return a serialized version of the pf_addr */
	std::string GetStr() const;

private:
	void Init(uint32_t adr0, uint32_t adr1, uint32_t adr2, uint32_t adr3);
};

template<>
inline Log::flux& Log::flux::operator<< <pf_addr> (pf_addr addr)
{
	_str += addr.GetStr();
	return *this;
}

#endif /* PF_ADDR_H */

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

#ifndef _HOST_H_
#define _HOST_H_

#include "key.h"
#include "jrb.h"
#include "mutex.h"
#include "dllist.h"
#include "pf_log.h"

#define SUCCESS_WINDOW 20
#define GOOD_LINK 0.8
#define BAD_LINK 0.3

class ChimeraHost;

class HostGlobal : public Mutex
{
	JRB hosts;
	Dllist free_list;
	size_t size;
	size_t max;

	unsigned long network_address(const char* hostname) const;

public:

	/** host_init:
	 ** initialize a host struct with a #size# element cache.
	 */
	HostGlobal(size_t size);

	/** host_get:
	 ** gets a host entry for the given host, getting it from the cache if
	 ** possible, or alocates memory for it
	 */
	ChimeraHost* GetHost(const char* hn, int port);

	/** host_decode:
	 ** decodes a string into a chimera host structure. This acts as a
	 ** host_get, and should be followed eventually by a host_release.
	 */
	ChimeraHost* DecodeHost (const char *s);

	/** host_release:
	 ** releases a host from the cache, declaring that the memory could be
	 ** freed any time. returns NULL if the entry is deleted, otherwise it
	 ** returns #host#
	 */
	void ReleaseHost(ChimeraHost* host);
};

class ChimeraHost
{
	std::string name;
	unsigned long address;
	int failed;
	double failuretime;
	int port;
	double latency;
	double loss;
	double success;
	int success_win[SUCCESS_WINDOW];
	int success_win_index;
	float success_avg;
	Key key;

public:

	ChimeraHost(const std::string& name, int port, unsigned long address);

	unsigned long GetAddress() const { return address; }

	/** host_encode:
	 ** encodes the #host# into a string, putting it in #s#, which has
	 ** #len# bytes in it.
	 */
	std::string Encode() const;

	/** host_update_stat:
	 ** updates the success rate to the host based on the SUCCESS_WINDOW average
	 */
	void UpdateStat (int success);

	const Key& GetKey() const { return key; }
	void SetKey(Key k) { key = k; }

	const std::string& GetName() const { return name; }
	int GetPort() const { return port; }
	double GetFailureTime() const { return failuretime; }
	double GetLatency() const { return latency; }

	void SetFailureTime(double f) { failuretime = f; }
	float GetSuccessAvg() const { return success_avg; }

};

template<>
inline Log::flux& Log::flux::operator<< <ChimeraHost> (ChimeraHost host)
{
	str += host.GetName() + ":" + TypToStr(host.GetPort());
	return *this;
}


#endif /* _HOST_H_ */

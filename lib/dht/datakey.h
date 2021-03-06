/*
 * Copyright(C) 2012 Benoît Saccomano
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

#ifndef DATAKEY_H
#define DATAKEY_H

#include "data.h"
#include <set>
#include <util/key.h>
#include <util/pf_log.h>

class DataKey : public Data
{
public:
	typedef std::set<Key> KeySet;

	DataKey(Key k);
	/** Create a DataKey from a serialized one */
	DataKey(char* buff);
	/** Add a key in the keyList and update the updateTime parameter */
	void add(Key k);
	/** Remove a key from the keyList */
	void remove(Key k);
	/** @return the number of elements in the data */
	size_t getSize() const;
	/** @return the type of the data */
	DataType getDataType() const;
	/** @return true if the data is empty */
	bool isEmpty() const;
	/** Serialize the keyList in binary format */
	void dump(char* buff) const;
	/** @return the size of the seralised data */
	size_t getSerialisedSize() const;
	std::string GetStr() const;

	/** @return a const iterator at the beginning of the key collection */
	KeySet::const_iterator begin() const;
	/** @return a const iterator at the end of the key collection */
	KeySet::const_iterator end() const;

private:
	KeySet keySet_;
};

template<>
inline Log::flux& Log::flux::operator<< <DataKey> (DataKey dk)
{
	_str += dk.GetStr();
	return *this;
}


#endif

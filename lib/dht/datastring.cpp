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

#include "datastring.h"
#include <util/time.h>
#include <net/netutil.h>

DataString::DataString(std::string name):Data()
{
	nameSet_.insert(name);
}

DataString::DataString(char* buff)
{
	uint32_t s = Netutil::ReadInt32(buff);
	buff += Netutil::getSerialisedSize(s);
	for (uint32_t i=0 ; i < s; i++)
	{
		std::string str = Netutil::ReadStr(buff);
		nameSet_.insert(str);
		buff += Netutil::getSerialisedSize(str);
	}
}

void DataString::add(std::string name)
{
	nameSet_.insert(name);
	updateTime_=time::dtime();
}

void DataString::remove(std::string name)
{
	nameSet_.erase(name);
}

size_t DataString::getSize() const
{
	return nameSet_.size();
}

DataType DataString::getDataType() const
{
	return STRING_LIST;
}

bool DataString::isEmpty() const
{
	return nameSet_.empty();
}

void DataString::dump(char* buff) const
{
	/* Type */
	uint32_t type = (uint32_t) this->getDataType();
	Netutil::dump(type, buff);
	buff += Netutil::getSerialisedSize(type);

	/* Number of strings */
	uint32_t s = (uint32_t) this->getSize();
	Netutil::dump(s, buff);
	buff += Netutil::getSerialisedSize(s);

	/* Strings */
	NameSet::const_iterator it;
	for (it=nameSet_.begin() ; it != nameSet_.end(); it++)
	{
		Netutil::dump(*it, buff);
		buff += Netutil::getSerialisedSize(*it);
	}
}

size_t DataString::getSerialisedSize() const
{
	size_t s = sizeof(uint32_t); /* Type */
	s += sizeof(uint32_t); /* Number of strings */

	/* Strings */
	NameSet::const_iterator it;
	for (it=nameSet_.begin() ; it != nameSet_.end(); it++)
		s += Netutil::getSerialisedSize(*it);

	return s;
}

std::string DataString::GetStr() const
{
	std::string str;
	NameSet::const_iterator it;
	for (it=nameSet_.begin() ; it != nameSet_.end(); it++)
	{
		if(str.size())
			str += ",";
		str += *it;
	}
	return "Strings[" + str + "]";
}

DataString::NameSet::const_iterator DataString::begin() const
{
	return nameSet_.begin();
}

DataString::NameSet::const_iterator DataString::end() const
{
	return nameSet_.end();
}

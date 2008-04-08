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

#include <map>
#include <string>
#include <fstream>
#include <stdlib.h>
#include "session_config.h"
#include "log.h"

template<>
std::string SessionConfigValue<std::string>::GetAsString() const
{
	return value;
};

SessionConfig session_cfg;
SessionConfig tree_cfg;

SessionConfig::SessionConfig() : Mutex(RECURSIVE_MUTEX), filename("")
{
}

SessionConfig::~SessionConfig()
{
	Save();

	for(std::map<std::string, SessionConfigValueBase*>::iterator it = list.begin();
		it != list.end();
		++it)
	{
		delete it->second;
	}
}

static ssize_t getline(std::string& line, std::fstream& file)
{
	line.clear();
	std::getline(file, line);
	if(file.eof())
		return -1;
	return line.size();
}

void SessionConfig::Load(const std::string& _filename)
{
	BlockLockMutex lock(this);
	filename = _filename;

	std::fstream fin;
	fin.open(filename.c_str(), std::fstream::in);
	if(!fin)
		return;

	ssize_t read;
	int line_nbr;
	std::string line;

	while ((read = getline(line, fin)) >= 0)
	{
		line_nbr++;
		if(line.size() == 0 || line.at(0) == '#' )
			continue;

		std::string::size_type equ_pos = line.find('=',0);
		if(equ_pos == std::string::npos)
		{
			log[W_ERR] << "SessionConfig: Wrong format on line " << line_nbr;
			continue;
		}

		Parse(line);
	}

	fin.close();
	log[W_INFO] << "SessionConfig: Loaded " << filename;
}

void SessionConfig::Save()
{
	BlockLockMutex lock(this);
	std::fstream fout;
	fout.open(filename.c_str(), std::fstream::out);
	if(!fout)
	{
		log[W_ERR] << "SessionConfig: Unable to save config file to " << filename;
		return;
	}

	for(std::map<std::string, SessionConfigValueBase*>::iterator it = list.begin();
		it != list.end();
		++it)
	{
		fout << it->first << "=" << it->second->GetAsString() << std::endl;
	}
	fout.close();
	log[W_INFO] << "SessionConfig: Config saved in " << filename;
}

void SessionConfig::Parse(const std::string& line)
{
	BlockLockMutex lock(this);
	std::string::size_type equ_pos = line.find('=',0);
	if(equ_pos == std::string::npos)
		return;

	std::string opt = line.substr(0, equ_pos);
	std::string val = line.substr(equ_pos+1);

	// val is considered to be an int if it doesn't contain
	// a '.' (ip address have to be handled as string...
	if(val.find('.',0) == std::string::npos
		&& ((val.at(0) >= '0' && val.at(0) <= '9')))
	{
		uint32_t nbr = strtoul(val.c_str(), NULL, 10);
		Set(opt, nbr);
	}
	else
		Set(opt, val);
}

void SessionConfig::Display()
{
	BlockLockMutex lock(this);
	for(std::map<std::string, SessionConfigValueBase*>::iterator it = list.begin();
		it != list.end();
		++it)
	log[W_INFO] << it->first << ":" << it->second->GetAsString();
}

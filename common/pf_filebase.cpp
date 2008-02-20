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

#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include "pf_filebase.h"
#include "pf_dir.h"

FileEntryBase::FileEntryBase(std::string _name, DirEntry* _parent)
	: name(_name), parent(_parent)
{
	stat.mode = S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO;     /* Protection */
	stat.uid = getuid();        /* UID propri?taire */
	stat.gid = getgid();        /* GID propri?taire */
	stat.size = 0;              /* Taille totale en octets */
//	stat.blksize_t st_blksize;  /* Taille de bloc pour E/S */
//	stat.blkcnt_t  st_blocks;   /* Nombre de blocs allou?s */
	stat.atime = time(NULL);    /* Heure dernier acc?s */
	stat.mtime = time(NULL);    /* Heure derni?re modification */
	stat.ctime = time(NULL);    /* Heure dernier changement ?tat */
}

FileEntryBase::~FileEntryBase()
{

}

std::string FileEntryBase::GetFullName() const
{
	DirEntry* p = parent;
	std::string path = "";

	while(p)
	{
		path = p->GetName() + "/" + path;
		p = p->parent;
	}

	path = path + name;
	return path;
}

bool FileEntryBase::IsChildOf(const FileEntryBase* f) const
{
	DirEntry* p = parent;

	while(p && p != f) p = p->parent;

	return (p);
}

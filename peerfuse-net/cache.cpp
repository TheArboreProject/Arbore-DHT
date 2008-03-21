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

#include <stack>
#include <cstring>

#include "cache.h"
#include "log.h"
#include "tools.h"
#include "network.h"

Cache cache;

Cache::Cache()
			: tree("", NULL)
{
}

Cache::~Cache()
{
}

void Cache::Load(std::string hd_path)
{
	Lock();
	try
	{
		hdd.BuildTree(GetTree(), hd_path);
	}
	catch(...)
	{					  /* U G L Y, I think we MUST find a solution. */
		Unlock();
		throw;
	}
	Unlock();
}

FileEntry* Cache::Path2File(std::string path, std::string* filename)
{
	Lock();
	DirEntry* current_dir = &tree;

	std::string name;

	while((name = stringtok(path, "/")).empty() == false)
	{
		FileEntry* tmp = current_dir->GetFile(name);
		if(!tmp)
		{
			if(path.find('/') == std::string::npos && filename)
			{			  /* we are in last dir, but this file doesn't exist */
				*filename = name;
				Unlock();
				return current_dir;
			}
			/* we aren't in last dir, so the path isn't found. */
			Unlock();
			return NULL;
		}

		if(!(current_dir = dynamic_cast<DirEntry*>(tmp)))
		{
			/* This isn't a directory. */
			if(path.empty())
			{
				/* We are on last dir, so it is a file. */
				Unlock();
				return tmp;
			}
			/* it isn't a file in path, so the path isn't found. */

			Unlock();
			return NULL;
		}
	}

	Unlock();
	return current_dir;
}

Packet Cache::CreateMkFilePacket(FileEntry* f)
{
	Packet pckt(NET_MKFILE, net.GetMyID());
	pckt.SetArg(NET_MKFILE_PATH, f->GetFullName());
	pckt.SetArg(NET_MKFILE_MODE, f->stat.mode);
	pckt.SetArg(NET_MKFILE_UID, f->stat.uid);
	pckt.SetArg(NET_MKFILE_GID, f->stat.gid);
	pckt.SetArg(NET_MKFILE_SIZE, (uint64_t)f->stat.size);
	pckt.SetArg(NET_MKFILE_ACCESS_TIME, (uint32_t)f->stat.atime);
	pckt.SetArg(NET_MKFILE_MODIF_TIME, (uint32_t)f->stat.mtime);
	pckt.SetArg(NET_MKFILE_CREATE_TIME, (uint32_t)f->stat.ctime);

	return pckt;
}

Packet Cache::CreateRmFilePacket(FileEntry* f)
{
	Packet pckt(NET_RMFILE, net.GetMyID());
	pckt.SetArg(NET_RMFILE_PATH, f->GetFullName());

	return pckt;
}

FileList Cache::GetAllFiles()
{
	/* Stack used to store states of each directories */
	std::stack<std::pair<FileMap::const_iterator, FileMap::const_iterator> > stack;
	FileList list;

	Lock();
	DirEntry* current_dir = GetTree();

	/* GetFiles() returns a reference, so we can use it directly */
	FileMap::const_iterator it = current_dir->GetFiles().begin();
	FileMap::const_iterator end = current_dir->GetFiles().end();

	list.insert(GetTree());

	while(current_dir)
	{
		for(; it != end; ++it)
		{
			DirEntry* dir = dynamic_cast<DirEntry*>(it->second);

			list.insert(it->second);

			if(dir)
			{
				current_dir = dir;
				break;
			}
		}
		/* End of dir, we go back on top folder */
		if(it == end)
		{
			current_dir = current_dir->GetParent();
			if(current_dir)
			{
				/* Restore state of parent dir */
				std::pair<FileMap::const_iterator, FileMap::const_iterator> iterators = stack.top();
				it = iterators.first;
				end = iterators.second;
				stack.pop();
			}
		}
		/* We enter in a subdir */
		else
		{
			/* Store current state in stack */
			stack.push(std::pair<FileMap::const_iterator, FileMap::const_iterator>(++it, end));

			/* And change iterators to next dir */
			it = current_dir->GetFiles().begin();
			end = current_dir->GetFiles().end();
		}

	}

	Unlock();

	return list;
}

void Cache::MkFile(std::string path, pf_stat stat, Peer* sender)
{
	std::string filename;
	FileEntry* file = 0;

	try
	{
		Lock();

		file = Path2File(path, &filename);
		DirEntry* dir = dynamic_cast<DirEntry*>(file);

		if(!file)
		{
			Unlock();
			throw NoSuchFileOrDir();
		}

		if(filename.empty() || !dir)
		{
			Unlock();
			throw FileAlreadyExists();
		}

		if(stat.mode & S_IFDIR)
			file = new DirEntry(filename, dir);
		else
			file = new FileEntry(filename, dir);

		/* Copy stat only if this isn't my who created this file! */
		if(sender)
			file->stat = stat;

		dir->AddFile(file);

		log[W_DEBUG] << "Created " << (stat.mode & S_IFDIR ? "dir " : "file ") << filename << " in " << path << ". There are " << dir->GetSize() << " files and directories";

		try
		{
			hdd.MkFile(file);
		}
		catch(...)
		{
			Unlock();
			throw;
		}

		filedist.AddFile(file, sender);

		Unlock();

	}
	catch(Cache::FileAlreadyExists &e)
	{
		/* If it me who wants to create this file, we go out
		 * to let fuse returns an error.
		 */
		if(sender == NULL)
			throw;

		/* This file already exists, but do not panic! We take modifications only if
		 * this file is more recent than mine.
		 */
		// TODO: fuck that leaf = e.file;

		Lock();

		time_t dist_ts = stat.mtime;

		if(file->stat.mtime > dist_ts)
		{
			/* My file is more recent than peer's, so I send it a mkfile
			 * to correct this.
			 */
			Packet pckt = cache.CreateMkFilePacket(file);
			pckt.SetDstID(sender->GetID());
			sender->SendMsg(pckt);
			Unlock();
			return;
		}
		else if(file->stat.mtime == dist_ts)
		{
			/* TODO Same timestamp, what can we do?... */
			Unlock();
			return;			  /* same version, go out */
		}

		file->stat = stat;

		Unlock();
	}
}

void Cache::RmFile(std::string path, Peer* sender)
{
	Lock();
	FileEntry* f = Path2File(path);

	// If it's a dir that isn't empty -> return error
	if (f->stat.mode & S_IFDIR)
	{
		DirEntry* d = static_cast<DirEntry*>(f);
		if(d->GetSize() != 0)
		{
			Unlock();
			throw DirNotEmpty();
		}
	}

	if(!f)
	{
		Unlock();
		throw NoSuchFileOrDir();
	}

	if(!f->GetParent())
	{
		Unlock();
		throw NoPermission();
	}

	log[W_DEBUG] << "Removed " << path;

	try
	{
		hdd.RmFile(f);
	}
	catch(...)
	{
		Unlock();
		throw;
	}

	/* Send before removing file */
	filedist.RemoveFile(f, sender);

	f->GetParent()->RemFile(f);
	Unlock();

}

void Cache::ModFile(std::string path, Peer* sender)
{
	Lock();

	Unlock();
}



void Cache::ChOwn(std::string path, uid_t uid, gid_t gid)
{
	Lock();

	FileEntry* file = Path2File(path);
	if(!file)
	{
		Unlock();
		throw NoSuchFileOrDir();
	}

	file->stat.uid = uid;
	file->stat.gid = gid;

	Unlock();
}

void Cache::ChMod(std::string path, mode_t mode)
{
	Lock();

	FileEntry* file = Path2File(path);
	if(!file)
	{
		Unlock();
		throw NoSuchFileOrDir();
	}

	file->stat.mode = mode;

	Unlock();
}

pf_stat Cache::GetAttr(std::string path)
{
	pf_stat stat;

	Lock();

	FileEntry* file = Path2File(path);
	if(!file)
	{
		Unlock();
		throw NoSuchFileOrDir();
	}

	stat = file->stat;
	Unlock();

	return stat;
}

void Cache::FillReadDir(const char* path, void *buf, fuse_fill_dir_t filler,
	off_t offset, struct fuse_file_info *fi)
{
	Lock();
	DirEntry* dir = dynamic_cast<DirEntry*>(cache.Path2File(path));

	if(!dir)
	{
		Unlock();
		throw NoSuchFileOrDir();
	}

	FileMap files = dir->GetFiles();
	for(FileMap::const_iterator it = files.begin(); it != files.end(); ++it)
	{
		struct stat st;
		memset(&st, 0, sizeof st);
		/*st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;*/

		if(filler(buf, it->second->GetName().c_str(), &st, 0))
			break;
	}

	Unlock();

}

void Cache::UpdateRespFiles()
{
	Lock();
	filedist.UpdateRespFiles();
	Unlock();
}

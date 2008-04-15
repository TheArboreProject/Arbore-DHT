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

#ifndef FILE_CHUNK_H
#define FILE_CHUNK_H

#include <time.h>
#include <unistd.h>

class FileChunk
{
	time_t access_time;
	char* data;
	off_t offset;
	size_t size;

public:
	FileChunk() : access_time(0), data(NULL), offset(0), size(0) {}
	FileChunk(const char* _data, off_t _offset, size_t _size);
	FileChunk(const FileChunk &other);
	FileChunk& operator=(const FileChunk &other);
	~FileChunk();

	const time_t GetAccessTime() const { return access_time; }
	const off_t GetOffset() const { return offset; }
	const size_t GetSize() const { return size; }
	const char* GetData() const { return data; }

	void Merge(FileChunk chunk);
};
#endif						  /* FILE_CHUNK_H */

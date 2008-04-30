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

#include "file_content.h"
#include "peers_list.h"
#include "packet.h"
#include "cache.h"

void FileContent::NetworkRequestChunk(FileChunkDesc chunk)
{
	if(sharers.size() == 0)
	{
		log[W_DEBUG] << "Request for file refs";
		// We don't know who have which part of the file
		if(!waiting_for_sharers)
		{
			cache.RequestFileRefs(filename);
			waiting_for_sharers = true;
		}
	}
	else
		NetworkFlushRequests();
}

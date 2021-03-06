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
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * 
 */

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>

#include <util/pf_log.h>

#include "connection_nossl.h"

ConnectionNoSsl::ConnectionNoSsl(int _fd)
			: Connection(_fd)
{
}

ConnectionNoSsl::~ConnectionNoSsl()
{
}

void ConnectionNoSsl::SocketWrite()
{
	if(send(fd, write_buf, write_buf_size, 0) <= 0)
		throw WriteError(std::string(strerror(errno)));

	write_buf_size = 0;
	free(write_buf);
	write_buf = NULL;
}

void ConnectionNoSsl::SocketRead()
{
	const int buf_size = 128;
	char buf[buf_size];

	ssize_t received = 0;

	do
	{
		received = recv(fd, (void*)buf, buf_size, 0);
		if(received > 0)
		{
			read_buf = (char*)realloc(read_buf, read_buf_size + received);
			memcpy(read_buf + read_buf_size, buf, received);
			read_buf_size += received;
		}
		else if(received == 0)
		{
			throw RecvError("Peer disconnected");
		}
		else				  // received < 0
		{
			throw RecvError(strerror(errno));
		}
	}
	while(received == buf_size);
}

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

#ifndef PF_CONNECTION_SSL_H
#define PF_CONNECTION_SSL_H

#include <queue>
#include "connection.h"
#include "certificate.h"

class ConnectionSsl : public Connection
{
private:
	SSL* ssl;
	struct buf_t
	{
		char* buf;
		size_t size;
	};
	std::queue<struct buf_t> buf_queue;

public:
	ConnectionSsl(SSL* _ssl, int _fd) : Connection(_fd), ssl(_ssl) {}
	~ConnectionSsl();

	void SocketWrite();
	void SocketRead();

	Certificate GetCertificate();
};
#endif						  // PF_CONNECTION_SSL_H

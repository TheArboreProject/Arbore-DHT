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
 * $Id: connection_ssl.h 346 2008-02-18 20:18:55Z lds $
 */

#ifndef CONNECTION_NOSSL_H
#define CONNECTION_NOSSL_H
#include "connection.h"
#include <exception>

class ConnectionNoSsl : public Connection
{
	int fd;
public:
	class ConnectionError : public std::exception {};

	ConnectionNoSsl(int _fd);

	void Write(const char* buf, size_t size);
	int Read(char* buf, size_t size);
};

#endif // CONNECTION_NOSSL_H

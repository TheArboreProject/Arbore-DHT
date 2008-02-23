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

#ifndef PF_SSL_SSL_H
#define PF_SSL_SSL_H

#include <list>
#include <exception>
#include <openssl/ssl.h>
#include "pf_ssl.h"
#include "certificate.h"
#include "connection_ssl.h"

class SslSsl : public Ssl
{
	// Context: holds default SSL values to use
	SSL_CTX* server_ctx;
	SSL_CTX* client_ctx;

	Certificate cert;
	Certificate cacert;
	PrivateKey key;
public:
	class ConnectionError : public std::exception {};

	SslSsl();
	~SslSsl();

	void SetCertificate(Certificate _cert) { cert = _cert; }
	void SetCACertificate(Certificate _cacert) { cacert = _cacert; }
	void SetPrivateKey(PrivateKey _key) { key = _key; }

	void HandShake(int fd);
	ConnectionSsl* GetConnection(int fd);

	void Connect(int fd);
	void Close(Connection* conn);
	void CloseAll();
};
#endif						  // PF_SSLSSL_H

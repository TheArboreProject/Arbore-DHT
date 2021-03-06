/*
 * Copyright(C) 2008 Romain Bignon
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

#ifndef MESSAGES_H
#define MESSAGES_H

#include <net/packet_handler.h>

class Chimera;

class ChimeraMessage : public PacketHandlerBase
{
public:
	virtual void Handle (Chimera& chimera, const Host& sender, const Packet& pckt) = 0;
	HandlerType getType() { return HANDLER_TYPE_CHIMERA; }
};

enum
{
	CHIMERA_JOIN_ADDRESS
};
extern PacketType ChimeraJoinType;

enum
{
	CHIMERA_JOIN_ACK_ADDRESSES
};
extern PacketType ChimeraJoinAckType;

enum
{
	CHIMERA_UPDATE_ADDRESS
};
extern PacketType ChimeraUpdateType;

enum
{
	CHIMERA_PIGGY_ADDRESSES
};

extern PacketType ChimeraPiggyType;

enum
{
	CHIMERA_JOIN_NACK_ADDRESS
};
extern PacketType ChimeraJoinNAckType;

enum
{
	CHIMERA_PING_ME
};
extern PacketType ChimeraPingType;

enum
{
	CHIMERA_CHAT_MESSAGE
};
extern PacketType ChimeraChatType;

#endif /* CHIMERA_MESSAGES_H */

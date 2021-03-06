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
 */

#include <stdlib.h>
#include <string>
#include <iostream>

#include <net/packet.h>
#include <net/packet_handler.h>
#include <net/hosts_list.h>
#include <scheduler/scheduler.h>
#include <util/pf_log.h>
#include <util/tools.h>
#include <chimera/chimera.h>
#include <chimera/messages.h>

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " listen_port [boostrap_host:port]" << std::endl;
		return EXIT_FAILURE;
	}

	Key me(StrToTyp<uint32_t>(argv[1]));

	Chimera* chimera = new Chimera(NULL, StrToTyp<uint16_t>(argv[1]), me);

	pf_log.SetLoggedFlags("ALL", false);
	Scheduler::StartSchedulers(5);

	if(argc > 2)
	{
		Host host = hosts_list.DecodeHost(argv[2]);
		pf_log[W_INFO] << "Connecting to " << host;
		chimera->Join(host);
	}

	std::string s;
	while(std::getline(std::cin, s))
	{
		std::string keystr = stringtok(s, " ");
		Key key;
		key = keystr;

		Packet pckt(ChimeraChatType, chimera->GetMe().GetKey(), key);
		pckt.SetFlag(Packet::MUSTROUTE);
		pckt.SetArg(CHIMERA_CHAT_MESSAGE, s);
		pf_log[W_INFO] << "CHAT[" << key << "] " << s;
		if(!chimera->Route(pckt))
			pf_log[W_INFO] << "Either I'm the destination, or we failed to send the packet.";
	}

	return EXIT_SUCCESS;
}

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

#include <iostream>
#include <syslog.h>
#include <sys/time.h>
#include "tools.h"
#include "pf_log.h"

Log pf_log;

static struct
{
	uint32_t flag;
	int level;
	const char* s;
} all_flags[] =
{
	{ W_DEBUG,      LOG_DEBUG,   "DEBUG"      },
	{ W_PARSE,      LOG_DEBUG,   "PARSE"      },
	{ W_ROUTING,    LOG_DEBUG,   "ROUTING"    },
	{ W_DESYNCH,    LOG_WARNING, "DESYNCH"    },
	{ W_WARNING,    LOG_WARNING, "WARNING"    },
	{ W_ERR,        LOG_ERR,     "ERR"        },
	{ W_INFO,       LOG_NOTICE,  "INFO"       },
	{ W_DHT,        LOG_DEBUG,   "DHT"        },
	{ W_FILE,       LOG_NOTICE,  "FILE"       }
};

Log::flux::~flux()
{
	int i;

	if(!(_flag & pf_log.LoggedFlags()))
		return;

	for(i = (sizeof all_flags / sizeof *all_flags) - 1; i >= 0 && !(_flag & all_flags[i].flag); --i)
		;

	if(i < 0)
		syslog(LOG_WARNING, "[SYSLOG] (%X) Unable to find how to log this message: %s", (uint32_t)_flag, _str.c_str());
	else
	{
		if((all_flags[i].level == LOG_ERR || all_flags[i].level == LOG_WARNING) && pf_log.ToSyslog())
			syslog(all_flags[i].level, "[%s] %s", all_flags[i].s, _str.c_str());

		struct timeval t;
		gettimeofday(&t, NULL);
		pf_log.std_lock.Lock();
		(all_flags[i].level == LOG_ERR ? std::cerr : std::cout)
			<< (t.tv_sec % (24 * 60 * 60)) / (60 * 60)
			<< ":" << (t.tv_sec % (60 * 60)) / 60
			<< ":" << t.tv_sec % 60
			<< "." << t.tv_usec / 1000
			<< "[" << all_flags[i].s << "] " << _str << std::endl;
		pf_log.std_lock.Unlock();
	}
}

Log::Log()
			: logged_flags(DEFAULT_LOGGED_FLAGS)
{
	openlog("peerfuse", LOG_CONS, LOG_DAEMON);
}

Log::~Log()
{
	closelog();
}

void Log::SetLoggedFlags(std::string s, bool _to_syslog)
{
	std::string token;

	to_syslog = _to_syslog;
	logged_flags = 0;

	while((token = stringtok(s, " ")).empty() == false)
	{
		int i;
		if(token == "ALL")
		{
			logged_flags = (uint32_t) -1;
			break;
		}

		for(i = (sizeof all_flags / sizeof *all_flags) - 1; i >= 0 && (token != all_flags[i].s); --i)
			;

		if(i >= 0)
			logged_flags |= all_flags[i].flag;
	}
}

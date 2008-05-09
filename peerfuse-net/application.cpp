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
 * This product includes cryptographic software written by Eric Younganus
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * $Id$
 */

#define FUSE_USE_VERSION 26

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/resource.h>

#include "libconfig.h"
#include "application.h"
#include "network.h"
#include "pf_ssl_ssl.h"
#include "certificate.h"
#include "crl.h"
#include "cache.h"
#include "log.h"
#include "pf_file.h"
#include "session_config.h"
#include "scheduler.h"
#include "hdd.h"
#include "content_list.h"

#ifndef PF_SERVER_MODE
#include <fuse.h>
#include "pf_fuse.h"
#endif

void* fuse_init(struct fuse_conn_info* fuse_info)
{
	Application::StartThreads();
	return NULL;
}

void fuse_destroy(void*)
{
	Application::Exit();
}

Application::Application()
{
	ConfigSection* section = conf.AddSection("listen", "Listening server", false);
	section->AddItem(new ConfigItem_string("bind", "IP Binding"));
	section->AddItem(new ConfigItem_int("port", "Port", 1, 65535));

	section = conf.AddSection("connection", "Connect to this server at launch", true);
	section->AddItem(new ConfigItem_string("host", "Hostname"), true);
	section->AddItem(new ConfigItem_int("port", "Port", 1, 65535));

	section = conf.AddSection("ssl", "SSL parameters", false);
	section->AddItem(new ConfigItem_string("cert", "Certificate path"));
	section->AddItem(new ConfigItem_string("key", "Private key path"));
	section->AddItem(new ConfigItem_string("ca", "CA certificate"));
	section->AddItem(new ConfigItem_bool("disable_crl", "Disable CRL download / check", "true"));
	section->AddItem(new ConfigItem_string("crl_url", "URL of the crl", "http://"));

	section = conf.AddSection("logging", "Log informations", false);
	section->AddItem(new ConfigItem_string("level", "Logging level"));
	section->AddItem(new ConfigItem_bool("to_syslog", "Log error and warnings to syslog"));

	section = conf.AddSection("hdd", "Harddisk configuration", false);
	section->AddItem(new ConfigItem_string("root", "Root directory of storage"));
	section->AddItem(new ConfigItem_string("workdir", "Root directory of config files"));
}

void Application::StartThreads()
{
	scheduler.Start();
	net.Start();
	content_list.Start();
}

void Application::Exit()
{
	content_list.Stop();
	net.Stop();
	scheduler.Stop();
	tree_cfg.Save();
	session_cfg.Save();
}

int Application::main(int argc, char *argv[])
{
	if(argc < 2 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
	{
		#ifdef PF_SERVER_MODE
		std::cerr << "Syntax: " << argv[0] << " <configpath>" << std::endl;
		#else
		std::cerr << "Syntax: " << argv[0] << " <configpath> <mount point> [fuse options]" << std::endl;
		#endif
		exit(EXIT_FAILURE);
	}

	struct rlimit rlim;
	if(!getrlimit(RLIMIT_CORE, &rlim) && rlim.rlim_cur != RLIM_INFINITY)
	{
		rlim.rlim_cur = RLIM_INFINITY;
		rlim.rlim_max = RLIM_INFINITY;
		setrlimit(RLIMIT_CORE, &rlim);
	}

	try
	{
		if(!conf.Load(argv[1]))
		{
			log[W_ERR] << "Unable to load configuration, exiting..";
			exit(EXIT_FAILURE);
		}
		log.SetLoggedFlags(conf.GetSection("logging")->GetItem("level")->String(), conf.GetSection("logging")->GetItem("to_syslog")->Boolean());

		session_cfg.Load(conf.GetSection("hdd")->GetItem("workdir")->String() + "/session.cfg");
		tree_cfg.Load(conf.GetSection("hdd")->GetItem("workdir")->String() + "/tree.cfg");

		cache.Load(conf.GetSection("hdd")->GetItem("root")->String());

		if(!conf.GetSection("ssl")->GetItem("disable_crl")->Boolean())
		{
			crl.Set(conf.GetSection("hdd")->GetItem("workdir")->String() + "/crl.pem",
				conf.GetSection("ssl")->GetItem("crl_url")->String());
			try
			{
				crl.Load();
			}
			catch(Crl::BadCRL &e)
			{
				log[W_ERR] << "Failed to load the crl: "<< e.GetString();
				exit(EXIT_FAILURE);
			}
			catch(...)
			{
				log[W_ERR] << "Failed to load the crl.";
				exit(EXIT_FAILURE);
			}
		}
		else
			crl.Disable();
		try
		{
			net.StartNetwork(&conf);
		}
		catch(Certificate::BadCertificate &e)
		{
			log[W_ERR] << "Unable to read certificate: " << e.GetString();
			exit(EXIT_FAILURE);
		}
		catch(Certificate::BadFile &e)
		{
			log[W_ERR] << "Unable to read certificate file";
			exit(EXIT_FAILURE);
		}
		catch(PrivateKey::BadPrivateKey &e)
		{
			log[W_ERR] << "Unable to read priveate key: " << e.GetString();
			exit(EXIT_FAILURE);
		}
		catch(PrivateKey::BadFile &e)
		{
			log[W_ERR] << "Unable to read private key file";
			exit(EXIT_FAILURE);
		}
		catch(Crl::BadCRL &e)
		{
			log[W_ERR] << "Unable to CRL file: " << e.GetString();
			exit(EXIT_FAILURE);
		}

		cache.UpdateRespFiles();

		umask(0);
		#ifndef PF_SERVER_MODE
		return fuse_main(argc-1, argv+1, &pf_oper, NULL);
		#else
		StartThreads();			  /* this function is called by fuse_main, so without fuse we call it ourselves. */
		while(1) sleep(1);

		// TODO: Catch SIGTERM
		// and do:
		Exit();
		#endif
	}
	catch(MyConfig::error_exc &e)
	{
		log[W_ERR] << "Error while loading:";
		log[W_ERR] << e.Reason();
	}
	catch(Thread::CantRun&e)
	{
		log[W_ERR] << "Unable to create network thread, exiting..";
	}
	catch(Network::CantOpenSock &e)
	{
		log[W_ERR] << "Unable to open socket, exiting..";
	}
	catch(Network::CantListen &e)
	{
		log[W_ERR] << "Unable to listen on port " << e.port << ", exiting..";
	}
	catch(Network::CantResolvHostname &e)
	{
		/* NOTE: This must never arrive because when I can't connect to server, I try
		 * an other server, or I wait alone..
		 */
		log[W_ERR] << "Unable to resolve hostname, exiting..";
	}
	catch(Hdd::HddAccessFailure &e)
	{
		log[W_ERR] << "Can't access cache root directory: " << e.dir;
	}
	catch(Hdd::HddWriteFailure &e)
	{
		log[W_ERR] << "Can't write to cache: " << e.dir;
	}

	exit(EXIT_FAILURE);

}

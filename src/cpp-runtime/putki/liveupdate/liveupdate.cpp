#include "liveupdate.h"

#if defined(LIVEUPDATE_ENABLE)

#include <putki/runtime.h>
#include <putki/pkgmgr.h>

#include <map>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <set>
#include <string>
#include <cstdlib>
#include <cstdio>

#include <string.h>

#if defined(USE_WINSOCK)
	#include <winsock2.h>
	#pragma comment(lib, "ws2_32.lib")
	#pragma comment(lib, "wsock32.lib")
#else
	#define closesocket close
#endif

#if !defined(_WIN32)
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <unistd.h>
#endif

namespace putki
{
	namespace
	{
		typedef std::map<instance_t, instance_t> RewriteMap;
		typedef std::map<std::string, instance_t> PathMap;
		RewriteMap s_rewrite;
		PathMap s_path2ptr;
	}

	namespace liveupdate
	{
		enum {
			READBUF_SIZE = 10*1024*1024
		};

		struct data
		{
			int socket;
			bool connected;

			char readbuf[READBUF_SIZE];
			unsigned long readpos;

			struct pkg_e
			{
				bool resolved;
				pkgmgr::loaded_package *pkg;
				pkgmgr::resolve_status *rs;
			};

			std::vector<pkg_e> pending;
			std::vector<pkg_e> stash;
			std::vector<pkg_e> endoftheline; // replaced stashes we won't use for resolving any more.
			
			std::set<std::string> askedfor;
		};

		void init()
		{
			#if defined(_WIN32)
				WSADATA wsaData;
				if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) 
				{
					std::cerr << "WSA init failure" << std::endl;
					return;
				}
			#endif
		}

		void hookup_object(instance_t ptr, const char *path)
		{
			if (!strcmp(path, "N/A"))
				return;
				
			// redirect the old if such exists.
			PathMap::iterator i = s_path2ptr.find(path);
			if (i != s_path2ptr.end())
			{
				s_rewrite[i->second] = ptr;
			}

			s_path2ptr[path] = ptr;
		}

		bool update_ptr(instance_t *ptr)
		{
			RewriteMap::const_iterator i = s_rewrite.find(*ptr);
			if (i != s_rewrite.end())
			{
				*ptr = i->second;
				return true;
			}
			return false;
		}

		void command(data *d, const char *str)
		{
			std::string cmd(str);
			cmd.append("\n");
			send(d->socket, cmd.c_str(), cmd.size(), 0);
		}

		data* connect()
		{
			data *d = new data();
			
			sockaddr_in addrLocal = {};
			addrLocal.sin_family = AF_INET;
			addrLocal.sin_port = htons(6788);
			addrLocal.sin_addr.s_addr = htonl(0x7f000001);
			d->socket = socket(AF_INET, SOCK_STREAM, 0);
			if (connect(d->socket, (sockaddr*)&addrLocal, sizeof(addrLocal)) < 0)
			{
				std::cerr << "Could not connect socket" << std::endl;
				closesocket(d->socket);
				delete d;
				return 0;
			}

			d->connected = true;
			d->readpos = 0;
			std::cout << "Connected to live update on socket " << d->socket << "!" << std::endl;

			char tmp[256];
			sprintf(tmp, "init %s", runtime::desc_str());
			command(d, tmp);

			command(d, "build haspointer");
			return d;
		}

		bool connected(data *d)
		{
			return d->connected;
		}

		void disconnect(data *d)
		{
			std::cout << "Disconnected from live update server." << std::endl;
			closesocket(d->socket);
			delete d;
		}

		bool same_root(pkgmgr::loaded_package *a, pkgmgr::loaded_package *b)
		{
			const char *A = pkgmgr::path_in_package_slot(a, 0);
			const char *B = pkgmgr::path_in_package_slot(b, 0);
			return A && B && !strcmp(A, B);
		}

		bool attempt_resolve_with_aux(data *d, data::pkg_e *target, data::pkg_e *source)
		{
			return !resolve_pointers_with(target->pkg, target->rs, source->pkg);
		}

		void on_package_stashed(data *d, data::pkg_e *p)
		{
			pkgmgr::free_resolve_status(p->rs);
			p->rs = 0;

			for (unsigned int i=0;;i++)
			{
				const char *objpath = pkgmgr::path_in_package_slot(p->pkg, i);
				if (objpath)
					std::cout << "Object [" << objpath << "] live-updated." << std::endl;
				else
					break;
			}
			pkgmgr::register_for_liveupdate(p->pkg);
		}

		void process_pending(data *d)
		{
			if (d->pending.empty())
				return;	

			// this function tail-recurses whenever it made some progress.

			// attempt cross-resolve.
			for(unsigned int i=0;i<d->pending.size();i++)
			{
				std::cout << "Processing package with " << pkgmgr::path_in_package_slot(d->pending[i].pkg, 0) << std::endl;

				// if there are no unresolved references, send directly.
				if (!pkgmgr::unresolved_reference(d->pending[i].pkg, 0))
				{
					std::cout << " -> No unresolved references." << std::endl;
					d->pending[i].resolved = true;
					continue;
				}

				for(unsigned int j=0;j<d->pending.size();j++)
				{
					if (i != j)
					{
						if (d->pending[j].resolved)
						{
							if (attempt_resolve_with_aux(d, &d->pending[i], &d->pending[j]))
							{
								d->pending[i].resolved = true;
								std::cout << "Resolved with pending package" << std::endl;
							}
						}
					}
				}

				// try with stash
				for(unsigned int j=0;j<d->stash.size();j++)
				{
					if (attempt_resolve_with_aux(d, &d->pending[i], &d->stash[j]))
					{
						std::cout << "Resolved from stash" << std::endl;
						d->pending[i].resolved = true;
					}
				}
				
				if (d->pending[i].resolved)
					std::cout << " -> It is now resolved." << std::endl;
			}

			// see if any packets are ready
			bool made_progress = false;
			
			for (int i=0;i<(int)d->pending.size();i++)
			{
				if (d->pending[i].resolved)
				{
					made_progress = true;
					std::cout << "Fully resolved package at slot " << i << ", moving to stash." << std::endl;

					// clean out stashed with same base object as this, move them to
					// the end of the line
					for (unsigned int k=0;k<d->stash.size();k++)
					{
						if (same_root(d->pending[i].pkg, d->stash[k].pkg))
						{
							// they've reached the end of the line!
							d->endoftheline.push_back(d->stash[k]);
							d->stash.erase(d->stash.begin()+k);
							k--;
						}
					}

					std::cout << "Erasing package " << i << std::endl;
					d->stash.push_back(d->pending[i]);
					d->pending.erase(d->pending.begin() + i);

					// resolve status is not needed any more.
					on_package_stashed(d, &d->stash.back());

					i--;
				}
				else
				{
					// make requests for all unresolved assets.
					for (unsigned int j=0;;j++)
					{
						const char *ref = pkgmgr::unresolved_reference(d->pending[i].pkg, j);
						if (!ref)
							break;

						if (!d->askedfor.count(ref))
						{
							char req[1024];
							sprintf(req, "build %s", ref);
							command(d, req);
							d->askedfor.insert(ref);
						}
					}
				}
			}
			
			if (made_progress)
				process_pending(d);
		}

		void on_recv(data *d)
		{
			// need at least this.
			if (d->readpos < 5)
				return;
			
			unsigned long sz = 0;

			for (int i=0;i<4;i++)
				sz |= (((unsigned char)d->readbuf[i+1]) << 8*i);

			if (d->readpos >= (sz + 5))
			{
				data::pkg_e pe;
				pe.rs = pkgmgr::alloc_resolve_status();

				char *buf = new char[sz];
				memcpy(buf, &d->readbuf[5], sz);

				pe.pkg = pkgmgr::parse(buf, buf + sz, pe.rs);
				pkgmgr::free_on_release(pe.pkg);

				pe.resolved = false;
				if (!pe.pkg)
				{
					std::cerr << "! Read broken package !" << std::endl;
					pkgmgr::free_resolve_status(pe.rs);
				}
				else
				{
				
					for (unsigned int i=0;;i++)
					{
						const char *objpath = pkgmgr::path_in_package_slot(pe.pkg, i);
						if (!objpath)
							break;
						std::cout << " slot[" << i << "] is [" << objpath << "]" << std::endl;
					}
	
					bool replaced = false;
					for (unsigned int i=0;i<d->pending.size();i++)
					{
						if (same_root(d->pending[i].pkg, pe.pkg))
						{
							pkgmgr::release(d->pending[i].pkg);
							pkgmgr::free_resolve_status(d->pending[i].rs);
							d->pending[i] = pe;
							replaced = true;
							std::cout << "Cleaned out package" << std::endl;
						}
					}

					if (!replaced)
						d->pending.push_back(pe);

					process_pending(d);
				}

				// peel off this package.
				unsigned long peel = sz + 5;
				for (unsigned long bk=0;bk<(d->readpos-peel);bk++)
					d->readbuf[bk] = d->readbuf[bk + peel];
				d->readpos -= peel;

				on_recv(d);
			}
		}

		void update(data *d)
		{
			if (!d->connected)
				return;

			command(d, "poll");

			// try reading.
			fd_set fds;
			FD_ZERO(&fds);
			FD_SET(d->socket, &fds);

			timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 0;
			if (select(d->socket + 1, &fds, (fd_set *) 0, (fd_set *) 0, &tv))
			{
				int read;
				if ((read = recv(d->socket, &d->readbuf[d->readpos], READBUF_SIZE - d->readpos, 0)) > 0)
				{
					d->readpos += read;
					on_recv(d);
				}
				else
				{
					d->connected = false;
				}
			}
		}
	}
}

#endif

#include "liveupdate.h"

#if defined(LIVEUPDATE_ENABLE)

#include <putki/runtime.h>
#include <putki/pkgmgr.h>
#include <putki/config.h>
#include <putki/log/log.h>

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
	#include <netinet/in.h>
#endif

#define LIVEUPDATE_DEBUG(x) PTK_DEBUG(x)

namespace putki
{
	namespace
	{
		typedef unsigned int pathid_t;

		typedef std::map<instance_t, instance_t> RewriteMap;
		typedef std::map<pathid_t, instance_t> PathMap;
		typedef std::map<instance_t, pathid_t> PtrToPath;
		typedef std::map<std::string, pathid_t> PathToId;

		RewriteMap s_rewrite;
		PtrToPath s_ptr2path;
		PathMap s_path2ptr;
		PathToId s_path2id;
		pathid_t s_pathid_counter = 100;
	}

	namespace liveupdate
	{
		enum {
			READBUF_SIZE = 256*1024*1024
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
				PTK_WARNING("WSA init failure")
				return;
			}
			#endif
		}

		pathid_t get_path_id(const char *path)
		{
			PathToId::iterator i = s_path2id.find(path);
			if (i != s_path2id.end())
			{
				return i->second;
			}

			s_pathid_counter++;
			s_path2id.insert(PathToId::value_type(path, s_pathid_counter));
			return s_pathid_counter;
		}

		void hookup_object(instance_t ptr, const char *path)
		{
			if (!strcmp(path, "N/A")) {
				return;
			}

			pathid_t path_id = get_path_id(path);
			s_path2ptr[path_id] = ptr;
			s_ptr2path[ptr] = path_id;
		}

		bool update_ptr(instance_t *ptr)
		{
			PtrToPath::const_iterator i = s_ptr2path.find(*ptr);
			if (i != s_ptr2path.end())
			{
				PathMap::const_iterator j = s_path2ptr.find(i->second);
				if (j != s_path2ptr.end())
				{
					if (*ptr != j->second)
					{
						*ptr = j->second;
						return true;
					}
				}
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
			addrLocal.sin_port = htons(5556);
			addrLocal.sin_addr.s_addr = htonl(0x7f000001);
			d->socket = socket(AF_INET, SOCK_STREAM, 0);
			if (connect(d->socket, (sockaddr*)&addrLocal, sizeof(addrLocal)) < 0)
			{
				PTK_WARNING("Could not connect to live update socket")
				closesocket(d->socket);
				delete d;
				return 0;
			}

			d->connected = true;
			d->readpos = 0;
			PTK_DEBUG("Connected to live update on socket " << d->socket << "!");

			char tmp[256];
			sprintf(tmp, "init %s %s", runtime::desc_str(), get_build_config());
			command(d, tmp);
			return d;
		}

		bool connected(data *d)
		{
			return d->connected;
		}

		void disconnect(data *d)
		{
			PTK_WARNING("Disconnected from live update server.");
			closesocket(d->socket);
			delete d;
		}

		bool same_root(pkgmgr::loaded_package *a, pkgmgr::loaded_package *b)
		{
			const char *A = pkgmgr::path_in_package_slot(a, 0, true);
			const char *B = pkgmgr::path_in_package_slot(b, 0, true);
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

			for (unsigned int i=0;; i++)
			{
				const char *objpath = pkgmgr::path_in_package_slot(p->pkg, i, true);
				if (objpath)
				{
					LIVEUPDATE_DEBUG("Object [" << objpath << "] live-updated.")
				}
				else{
					break;
				}
			}
			pkgmgr::register_for_liveupdate(p->pkg);
		}

		void process_pending(data *d)
		{
			if (d->pending.empty()) {
				return;
			}

			// this function tail-recurses whenever it made some progress.

			// attempt cross-resolve.
			for(unsigned int i=0; i<d->pending.size(); i++)
			{
				LIVEUPDATE_DEBUG("Processing package with " << pkgmgr::path_in_package_slot(d->pending[i].pkg, 0, true))

				// if there are no unresolved references, send directly.
				if (!num_unresolved_slots(d->pending[i].pkg))
				{
					LIVEUPDATE_DEBUG(" -> No unresolved references.")
					d->pending[i].resolved = true;
					continue;
				}

				for(unsigned int j=0; j<d->pending.size(); j++)
				{
					if (i != j) {
						if (d->pending[j].resolved) {
							if (attempt_resolve_with_aux(d, &d->pending[i], &d->pending[j]))
							{
								d->pending[i].resolved = true;
								LIVEUPDATE_DEBUG("-> Resolved with pending package")
							}
						}
					}
				}

				// try with stash
				for(unsigned int j=0; j<d->stash.size(); j++)
				{
					if (attempt_resolve_with_aux(d, &d->pending[i], &d->stash[j]))
					{
						LIVEUPDATE_DEBUG("Resolved from stash")
						d->pending[i].resolved = true;
					}
				}

				if (d->pending[i].resolved) {
					LIVEUPDATE_DEBUG(" -> It is now resolved.")
				}
			}

			// see if any packets are ready
			bool made_progress = false;

			for (int i=0; i<(int)d->pending.size(); i++)
			{
				if (d->pending[i].resolved)
				{
					made_progress = true;
					LIVEUPDATE_DEBUG("Fully resolved package at slot " << i << ", moving to stash.")

					// clean out stashed with same base object as this, move them to
					// the end of the line
					for (unsigned int k=0; k<d->stash.size(); k++)
					{
						if (same_root(d->pending[i].pkg, d->stash[k].pkg))
						{
							// they've reached the end of the line!
							d->endoftheline.push_back(d->stash[k]);
							d->stash.erase(d->stash.begin()+k);
							k--;
						}
					}

					LIVEUPDATE_DEBUG("Erasing package " << i)
					d->stash.push_back(d->pending[i]);
					d->pending.erase(d->pending.begin() + i);

					// resolve status is not needed any more.
					on_package_stashed(d, &d->stash.back());

					i--;
				}
				else
				{
					int next = 0;
					while ((next = pkgmgr::next_unresolved_slot(d->pending[i].pkg, next)) >= 0)
					{
						const char *ref = pkgmgr::path_in_package_slot(d->pending[i].pkg, next, false);
						if (ref)
						{
							if (!d->askedfor.count(ref))
							{
								char req[1024];
								sprintf(req, "build %s", ref);
								command(d, req);
								d->askedfor.insert(ref);
							}
						}
						next++;
					}
				}
			}

			if (made_progress) {
				process_pending(d);
			}
		}

		void on_recv(data *d)
		{
			// need at least this.
			if (d->readpos < 16) {
				return;
			}
			
			uint32_t hdr_size, data_size;
			if (!pkgmgr::get_header_info(d->readbuf, d->readbuf + d->readpos, &hdr_size, &data_size))
				return;
				
			if (d->readpos < hdr_size + data_size)
				return;
				
			data::pkg_e pe;
			pe.rs = pkgmgr::alloc_resolve_status();

			char *buf = new char[data_size];
			memcpy(buf, &d->readbuf[hdr_size], data_size);
		
			pe.pkg = pkgmgr::parse(d->readbuf, buf, 0, pe.rs);
			
			pkgmgr::free_on_release(pe.pkg);

			pe.resolved = false;
			if (!pe.pkg)
			{
				LIVEUPDATE_DEBUG("! Read broken package !")
				pkgmgr::free_resolve_status(pe.rs);
			}
			else
			{

				for (unsigned int i=0;; i++)
				{
					const char *objpath = pkgmgr::path_in_package_slot(pe.pkg, i, false);
					if (!objpath) {
						break;
					}
					LIVEUPDATE_DEBUG(" slot[" << i << "] is [" << objpath << "]")
				}

				bool replaced = false;
				for (unsigned int i=0; i<d->pending.size(); i++)
				{
					if (same_root(d->pending[i].pkg, pe.pkg))
					{
						pkgmgr::release(d->pending[i].pkg);
						pkgmgr::free_resolve_status(d->pending[i].rs);
						d->pending[i] = pe;
						replaced = true;
						LIVEUPDATE_DEBUG("Cleaned out package")
					}
				}

				if (!replaced) {
					d->pending.push_back(pe);
				}

				process_pending(d);
			}

			// peel off this package.
			unsigned long peel = hdr_size + data_size;
			for (unsigned long bk=0; bk<(d->readpos-peel); bk++)
				d->readbuf[bk] = d->readbuf[bk + peel];
			d->readpos -= peel;

			on_recv(d);

		}

		void update(data *d)
		{
			if (!d->connected) {
				return;
			}

			command(d, "poll");

			// try reading.
			fd_set fds;
			FD_ZERO(&fds);
			FD_SET(d->socket, &fds);

			timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 0;
			while (d->connected && select(d->socket + 1, &fds, (fd_set *) 0, (fd_set *) 0, &tv))
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

				if (d->readpos == READBUF_SIZE)
				{
					d->connected = false;
					PTK_WARNING("Read buffer filled up! Must disconnect.")
					break;
				}
			}
		}
	}
}

#endif

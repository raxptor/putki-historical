#include <putki/liveupdate/liveupdate.h>

#include <putki/builder/db.h>
#include <putki/builder/builder.h>
#include <putki/builder/app.h>
#include <putki/builder/source.h>
#include <putki/builder/build.h>
#include <putki/builder/package.h>

#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <set>

#if defined(USE_WINSOCK)
	#include <winsock2.h>
	#pragma comment (lib, "wsock32.lib")
	#pragma comment (lib, "ws2_32.lib")
#else
	#include <pthread.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
#endif

namespace putki
{

	namespace liveupdate
	{
#if defined(USE_WINSOCK)

		struct data
		{
			SOCKET socket;
			CRITICAL_SECTION _cr0;
			std::vector<std::string> _assets_updates;
			db::data *source_db;
		};
		
		data* start_server(db::data *use_this_db)
		{
			data *d = new data();
			
			sockaddr_in addrLocal = {};
			
			addrLocal.sin_family = AF_INET;
			addrLocal.sin_port = htons(5566);
			addrLocal.sin_addr.s_addr = htonl(0x7f000001);
			
			d->socket = socket(AF_INET, SOCK_STREAM, 0);
			if (bind(d->socket, (sockaddr*)&addrLocal, sizeof(addrLocal)) < 0)
			{
				std::cerr << "Could not open listening socket" << std::endl;
				return 0;
			}

			d->source_db = use_this_db ? use_this_db : db::create();
			InitializeCriticalSection(&d->_cr0);
			listen(d->socket, 16);

			return d;
		}

		void enter_lock(data *lu)
		{
			EnterCriticalSection(&lu->_cr0);
		}

		void leave_lock(data *lu)
		{
			LeaveCriticalSection(&lu->_cr0);
		}

		void stop_server(data *which)
		{
			closesocket(which->socket);
			delete which;
		}

		int accept(data *d)
		{
			sockaddr_in client;
			int sz = sizeof(client);
			return accept(d->socket, (sockaddr*)&client, &sz);
		}

		int read(int socket, char *buf, unsigned long sz)
		{
			return recv(socket, buf, (int)sz, 0);
		}

		void close(int socket)
		{
			closesocket(socket);
		}

#else
		// null implementation
		struct data {
			int socket;
			pthread_mutex_t mtx;
			std::vector<std::string> _assets_updates;
			db::data *source_db;
		};
		
		data* start_server(db::data *use_this_db)
		{
			int s = socket(AF_INET, SOCK_STREAM, 0);
			if (s < 0)
			{
				std::cerr << "Could not open listening socket" << std::endl;
				return 0;
			}
			
			int optval = 1;
			setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
			
			sockaddr_in srv;
			srv.sin_family = AF_INET;
			srv.sin_addr.s_addr = INADDR_ANY;
			srv.sin_port = htons( 6788 );
			if (bind(s, (sockaddr*)&srv, sizeof(srv)) < 0)
			{
				std::cerr << "Binding failed." << std::endl;
				close(s);
				return 0;
			}
			
			listen(s, 10);
			
			data* d = new data();
			d->socket = s;
			d->source_db = use_this_db ? use_this_db : db::create();

			pthread_mutex_init(&d->mtx, 0);
			std::cout << "Started listening for live updates on socket " << s << std::endl;
			return d;
		}

		void enter_lock(data *d)
		{
			pthread_mutex_lock(&d->mtx);
		}

		void leave_lock(data *d)
		{
			pthread_mutex_unlock(&d->mtx);
		}
		
		int accept(data *d)
		{
			sockaddr_in client;
			socklen_t sz = sizeof(client);			
			return accept(d->socket, (sockaddr*)&client, &sz);
		}
		
		void stop_server(data *d)
		{
			close(d->socket);
			delete d;
		}
		
		void send_update(data *lu, db::data *data, const char *path) { }
#endif

		struct db_merge : public db::enum_i 
		{
			db::data *_output;
			void record(const char *path, type_handler_i *th, instance_t i)
			{
				db::insert(_output, path, th, i);
			}
		};
		
		void send_update(data *lu, const char *path)
		{
			enter_lock(lu);
			lu->_assets_updates.push_back(path);
			leave_lock(lu);
		}

		// These are all-platform stuff.
		void service_client(data *lu, const char *sourcepath, int skt)
		{
			char parsebuf[4096];
			char buf[256];
			int bytes;
			int parse = 0;
			
			std::queue<std::string> lines;
			int accepted_updates = 0;

			builder::data *builder = 0;
			runtime::descptr rt;
			
			db::data *input = db::create();
			
			
			while ((bytes = recv(skt, buf, sizeof(buf), 0)) > 0)
			{
				if (bytes + parse > sizeof(parsebuf)-1)
				{
					std::cerr << "Client filled parse buffer." << std::endl;
					close(skt);
					return;
				}
				memcpy(&parsebuf[parse], buf, bytes);
				parse += bytes;
				
				for (int i=0;i<parse;i++)
				{
					if (parsebuf[i] == 0xD || parsebuf[i] == 0xA)
					{
						parsebuf[i] = 0;
						if (i > 0)
							lines.push(parsebuf);
						
						const int count = i+1;
						for (int j=0;j<(parse-count);j++)
							parsebuf[j] = parsebuf[j+count];
							
						parse -= count;
						i = -1;
						continue;
					}
				}
				
				while (!lines.empty())
				{
					std::string cmd = lines.front();
					std::string arg0;
					
					int del = cmd.find_first_of(' ');
					if (del != std::string::npos)
					{
						arg0 = cmd.substr(del+1, cmd.size() - del);
						cmd = cmd.substr(0, del);
					}
					
					std::cout << "client [" << cmd << "] arg0 [" << arg0 << "]" << std::endl;
					if (cmd == "init")
					{
						// see what runtime it is.
						for (int i=0;;i++)
						{
							runtime::descptr p = runtime::get(i);
							if (!p)
								break;
							if (!strcmp(arg0.c_str(), runtime::desc_str(p)))
								rt = p;
						}

						if (!builder)
						{
							builder = builder::create(rt, sourcepath);
							if (builder)
							{
								std::cout << "Created builder for client." << std::endl;
							}
						}
					}

					std::vector<std::string> buildforclient;

					if (cmd == "poll" && builder)
					{
						std::set<std::string> already;
						enter_lock(lu);
						while (accepted_updates < (int)lu->_assets_updates.size())
						{
							const char *path = lu->_assets_updates[accepted_updates++].c_str();
							if (!already.count(path))
							{
								// need to be found in any of the output dbs.
								buildforclient.push_back(path);
								already.insert(path);
							}
						}

						leave_lock(lu);
					}

					if (cmd == "build")
					{
						buildforclient.push_back(arg0);
					}

					while (builder && !buildforclient.empty())
					{
						std::string & tobuild = buildforclient.front();

						enter_lock(lu);

						// load asset into source db if missing.
						type_handler_i *th;
						instance_t obj;
						if (!db::fetch(lu->source_db, tobuild.c_str(), &th, &obj))
						{
							std::string file_path = sourcepath + tobuild;
							std::cout << "Loading into source db because it is missing [" << file_path << "]" << std::endl;
							load_file_into_db(sourcepath, tobuild.c_str(), lu->source_db, true);
						}

						if (!db::fetch(lu->source_db, tobuild.c_str(), &th, &obj))
						{
							// 
							std::cout << "Failed to resolve path." << std::endl;
							leave_lock(lu);
							buildforclient.erase(buildforclient.begin());
							continue;
						}

						db::data *output = db::create();

						std::cout << "Sending to client [" << tobuild << "]" << std::endl;
							
						builder::build_source_object(builder, lu->source_db, tobuild.c_str(), output);
							
						build::post_build_ptr_update(lu->source_db, output);

						// should be done with this now.
						leave_lock(lu);
							
						package::data *pkg = package::create(output);
						package::add(pkg, tobuild.c_str(), true);
							
						const unsigned int sz = 10*1024*1024;
						char *buf = new char[sz];
						long bytes = package::write(pkg, rt, buf, sz);
							
						std::cout << "Got a package of " << bytes << " bytes for you." << std::endl;
							
						char pkttype = 1;
						send(skt, &pkttype, 1, 0);
						for (int i=0;i<4;i++)
						{
							char sz = (bytes >> (i*8)) & 0xff;
							send(skt, &sz, 1, 0);
						}
						send(skt, buf, bytes, 0);

						db::free(output);
						delete [] buf;

						buildforclient.erase(buildforclient.begin());
					}
					
					lines.pop();
				}
			}
			
			if (builder)
				builder::free(builder);
				
			db::free(input);
		}

	}

}

#include <putki/liveupdate/liveupdate.h>

#include <putki/builder/db.h>
#include <putki/builder/builder.h>
#include <putki/builder/app.h>
#include <putki/builder/source.h>
#include <putki/builder/build.h>
#include <putki/builder/package.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <queue>

namespace putki
{

	namespace liveupdate
	{
#if defined(_WIN32)

		struct data
		{

		};
		
		data* start_server(const char *name)
		{
			return new data();
		}

		void stop_server(data *which)
		{
			delete which;
		}

		void send_update(data *lu, db::data *data, const char *path)
		{

		}

#else
		// null implementation
		struct data {
			int socket;
		};
		
		data* start_server()
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
			std::cout << "Started listening for live updates on socket " << s << std::endl;
			return d;
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

		void service_client(const char *sourcepath, int skt)
		{
			char parsebuf[4096];
			char buf[256];
			int bytes;
			int parse = 0;
			
			std::queue<std::string> lines;
			putki::builder::data *builder = 0;
			putki::runtime rt;
			
			db::data *input = db::create();
			
			
			while ((bytes = read(skt, buf, sizeof(buf))) > 0)
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
						rt = (putki::runtime) atoi(arg0.c_str());
						if (!builder)
						{
							builder = putki::builder::create(rt);
							if (builder)
							{
								app_register_handlers(builder);
								std::cout << "Created builder for client." << std::endl;
							}
						}
					}
					
					if (cmd == "build")
					{
						if (builder)
						{
							// build asset.
							db::data *output = db::create();
							
							std::string file_path = sourcepath + arg0;
							putki::load_file_into_db(sourcepath, arg0.c_str(), input, true);
							
							putki::builder::build_source_object(builder, input, arg0.c_str(), output);
							
							// we aren't going to be domain switching any pointers here since
							// the client will have to request any updated assets.
							// instead just package the built assets and let the client.
							
							putki::package::data *pkg = putki::package::create(output);
							putki::package::add(pkg, arg0.c_str(), true);
							
							const unsigned int sz = 10*1024*1024;
							char *buf = new char[sz];
							long bytes = putki::package::write(pkg, rt, buf, sz);
							
							std::cout << "Got a package of " << bytes << " bytes for you." << std::endl;
							
							char pkttype = 1;
							send(skt, &pkttype, 1, 0);
							for (int i=0;i<4;i++)
							{
								char sz = (bytes >> (i*8)) & 0xff;
								send(skt, &sz, 1, 0);
							}
							send(skt, buf, bytes, 0);
						}
					}
					
					lines.pop();
				}
			}
			
			if (builder)
				putki::builder::free(builder);
				
			db::free(input);
		}

	}

}

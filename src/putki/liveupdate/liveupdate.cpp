#include <putki/liveupdate/liveupdate.h>

#include <putki/builder/db.h>
#include <putki/builder/builder.h>
#include <putki/builder/log.h>
#include <putki/builder/source.h>
#include <putki/builder/build.h>
#include <putki/builder/write.h>
#include <putki/builder/build-db.h>
#include <putki/builder/package.h>
#include <putki/builder/log.h>
#include <putki/sys/thread.h>
#include <putki/sys/sstream.h>

#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <map>

#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <netinet/in.h>

namespace putki
{

	namespace liveupdate
	{
		int skt_listen(int port)
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
			srv.sin_port = htons(port);
			if (bind(s, (sockaddr*)&srv, sizeof(srv)) < 0)
			{
				std::cerr << "Binding failed." << std::endl;
				close(s);
				return 0;
			}

			listen(s, 10);
			return s;
		}
		
		int skt_accept(int lp)
		{
			sockaddr_in client;
			socklen_t sz = sizeof(client);
			return accept(lp, (sockaddr*)&client, &sz);
		}
		
		struct edit
		{
			std::string data;
			int version;
			int sent_version;
		};
		
		typedef std::map<std::string, edit> edits_t;
		
		struct ed_session
		{
			std::string name;
			
			sys::mutex mtx;
			sys::condition cond;
			
			bool finished;
			bool ready;
			
			edits_t edits;
			int num_edits;
		};
		
		// Editor uses this
		struct ed_client
		{
			ed_session session;
			int socket;
		};
		
		ed_client *create_editor_connection()
		{
			ed_client *c = new ed_client();
			c->session.finished = false;
			c->session.ready = false;
			c->session.num_edits = 0;
			return c;
		}
		
		void* editor_read_thread(void *thr)
		{
			ed_client *cl = (ed_client *) thr;
			
			while (true)
			{
				sys::scoped_maybe_lock lk(&cl->session.mtx);
				int skt = cl->socket;
				while (skt == -1)
				{
					cl->session.cond.wait(&cl->session.mtx);
					skt = cl->socket;
				}
				
				lk.unlock();

				if (skt == -2)
				{
					APP_INFO("Exiting thread")
					return 0;
				}
				
				APP_INFO("Reading back data on socket " << cl->socket);
				
				char buf[256];
				int rd;
				while ((rd = read(skt, buf, sizeof(buf))) > 0)
				{
					// throw away data.
				}
				
				APP_INFO("Connection closed!")

				sys::scoped_maybe_lock lk2(&cl->session.mtx);
				cl->session.ready = false;
				cl->session.cond.broadcast();
			}
		}
		
		// called from the editor.
		void editor_on_edited(ed_client *conn, db::data *db, const char *path)
		{
			sys::scoped_maybe_lock lk(&conn->session.mtx);

			edits_t::iterator i = conn->session.edits.find(path);
			if (i == conn->session.edits.end())
			{
				edit ed;
				ed.version = 0;
				ed.sent_version = 0;
				conn->session.edits.insert(edits_t::value_type(path, ed));
				i = conn->session.edits.find(path);
			}
						
			type_handler_i *th;
			instance_t obj;
			if (!db::fetch(db, path, &th, &obj))
			{
				APP_WARNING("Edited object could not be read");
				return;
			}
			
			sstream tmp;
			write::write_object_into_stream(tmp, db, th, obj);
			const char *str = tmp.c_str();

			i->second.version++;
			i->second.data = std::string(str, str + tmp.size());
			conn->session.num_edits++;
			conn->session.cond.broadcast();
			APP_DEBUG("Inserted edit on " << path);
		}
		
		void run_editor_connection(ed_client *conn)
		{
			sockaddr_in addrLocal = {};
			addrLocal.sin_family = AF_INET;
			addrLocal.sin_port = htons(EDITOR_PORT);
			addrLocal.sin_addr.s_addr = htonl(0x7f000001);
			
			conn->socket = -1;
			
			sys::thread *read_thread = sys::thread_create(editor_read_thread, conn);
			
			while (true)
			{
				// notify read thread.
				conn->session.mtx.lock();
				conn->socket = -1;
				conn->session.cond.broadcast();
				conn->session.mtx.unlock();
		
				int skt = socket(AF_INET, SOCK_STREAM, 0);
				if (connect(skt, (sockaddr*)&addrLocal, sizeof(addrLocal)) < 0)
				{
					close(skt);
					usleep(50000);
					continue;
				}
				
				APP_INFO("Connected to live update server on socket " << skt);

				sys::scoped_maybe_lock lk(&conn->session.mtx);
				conn->socket = skt;
				conn->session.ready = true;
				
				sstream tmp;
				edits_t::iterator i = conn->session.edits.begin();
				while (i != conn->session.edits.end())
				{
					APP_INFO("The edit is on is " << i->first)
					i->second.sent_version = i->second.version;
					tmp << i->first << "\n";
					tmp << i->second.data << "\n";
					tmp << "<end>" << "\n";
					++i;
				}
				
				int wr = send(skt, tmp.c_str(), tmp.size(), 0);
				if (wr != tmp.size())
				{
					APP_INFO("Failed to write " << tmp.size() << " bytes, closing connection.")
					close(skt);
					continue;
				}
				
				// for socket thread
				conn->session.cond.broadcast();
			
				// connection loop.
				while (true)
				{
					APP_INFO("Waiting for edits")
					
					// wait either for closed socket or more edits
					int edits = conn->session.num_edits;
				
					while (conn->session.ready && edits == conn->session.num_edits)
						conn->session.cond.wait(&conn->session.mtx);
						
					APP_INFO("Num edits " << conn->session.num_edits)
						
					if (!conn->session.ready)
					{
						APP_INFO("Lost connection, retrying...")
						close(skt);
						conn->socket = -1;
						break;
					}
					
					tmp.clear();
					
					edits_t::iterator e = conn->session.edits.begin();
					while (e != conn->session.edits.end())
					{
						if (e->second.version != e->second.sent_version)
						{
							e->second.sent_version = e->second.version;
							tmp << e->first << "\n";
							tmp << e->second.data << "\n";
							tmp << "<end>\n";
						}
						e++;
					}
					
					
					int write = send(skt, tmp.c_str(), tmp.size(), 0);
					if (write != tmp.size())
					{
						APP_INFO("Failed to write " << tmp.size() << " bytes, closing connection.")
						close(skt);
						conn->socket = -1;
						// retry
						break;
					}
				}
			}
			
			if (conn->socket >= 0)
			{
				APP_INFO("Shutdown.")
				sys::scoped_maybe_lock lk(&conn->session.mtx);
				close(conn->socket);
				conn->socket = -2;
			}
			
			sys::thread_free(read_thread);
		}
		
		void release_editor_connection(ed_client *conn)
		{
		
		}
		
		struct data
		{
			std::vector<ed_session*> editors;
		};
		
		data *create()
		{
			return new data();
		}
		
		void free(data *d)
		{
			delete d;
		}
		
		struct thr_info
		{
			data *d;
			int socket;
		};
		
		// called on the server side from network connection.
		void editor_on_object(ed_session *session, const char *path, sstream *stream)
		{
			sys::scoped_maybe_lock lk(&session->mtx);

			APP_INFO("Got object [" << path << "] as follows. I had " << session->num_edits << " edits")
//			APP_INFO(stream->c_str())
			
			edits_t::iterator i = session->edits.find(path);
			if (i != session->edits.end())
			{
				const char *data = stream->c_str();
				i->second.version++;
				i->second.data = std::string(data, data + stream->size());
			}
			else
			{
				const char *data = stream->c_str();
				// insert new edits
				edit e;
				e.version = 1;
				e.data = std::string(data, data + stream->size());
				session->edits.insert(edits_t::value_type(path, e));
			}
			
			session->num_edits++;
			session->cond.broadcast();
		}
		
		void* editor_client_thread(void *arg)
		{
			thr_info *ptr = (thr_info *) arg;
			
			sstream name;
			name << "psd-" << (ptr->socket) << ptr;
			
			APP_INFO("Editor session created on socket " << ptr->socket << " with name " << name.c_str())
			
			data *d = ptr->d;
			
			ed_session *ed = new ed_session();
			ed->finished = false;
			ed->ready = false;
			ed->num_edits = 0;
			d->editors.push_back(ed);
			
			char buf[65536];
			
			sstream tmp;
			sstream *objbuf = 0;
			std::string obj;
			
			int rd;
			int readpos = 0;
			int parsed = 0, scanned = 0;
			while ((rd = read(ptr->socket, &buf[readpos], sizeof(buf)-readpos)) > 0)
			{
				readpos += rd;
				
				for (;scanned!=readpos;scanned++)
				{
					if (buf[scanned] == 0xd || buf[scanned] == 0xa)
					{
						buf[scanned] = 0x0;
						const char *line = &buf[parsed];
						
						// ignore empty lines
						if (scanned == parsed)
						{
							parsed++;
							continue;
						}
						
						if (!objbuf)
						{
							if (!strcmp(line, "<ready>"))
							{
								sys::scoped_maybe_lock(&ed->mtx);
								ed->ready = true;
								ed->cond.broadcast();
								APP_INFO("Editor session is ready")
							}
							else
							{
								obj = line;
								objbuf = &tmp;
								APP_INFO("Receiving object [" << obj << "]")
							}
						}
						else
						{
							if (!strcmp(line, "<end>"))
							{
								editor_on_object(ed, obj.c_str(), objbuf);
								objbuf->clear();
								objbuf = 0;
							}
							else
							{
								(*objbuf) << line << "\n";
							}
						}
						
						parsed = scanned + 1;
					}
				}
				
				for (int i=parsed;i!=readpos;i++)
				{
					buf[i-parsed] = buf[i];
				}
				
				scanned -= parsed;
				readpos -= parsed;
				parsed = 0;
				
				if (readpos == sizeof(buf))
				{
					APP_INFO("Read buffer overflowed. Malfunction in communication.");
					break;
				}
				
				APP_INFO("Readpos " << readpos << " parsed:" << parsed << " scanned:" << scanned)
			}
			
			APP_INFO("Editor session " << name.c_str() << " finished.")
			close(ptr->socket);
			
			ed->finished = true;
			ed->ready = true;
			
			delete ptr;
			return 0;
		}
		
		void* editor_listen_thread(void *arg)
		{
			data *d = (data *)arg;
			int s = skt_listen(EDITOR_PORT);
			
			APP_INFO("Started live update service, waiting for connections.");
			
			while (true)
			{
				thr_info *inf = new thr_info();
				inf->socket = skt_accept(s);
				inf->d = d;
				sys::thread_create(&editor_client_thread, inf);
			}
			return 0;
		}
		
		void run_server(data *d)
		{
			sys::thread *thr = sys::thread_create(editor_listen_thread, d);
			sys::thread_join(thr);
		}
	}

/*
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
			std::cout << "Registered update on " << path << "!" << std::endl;
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
			runtime::descptr rt = 0;
			std::string config = "Default";

			sys::mutex db_mtx, output_db_mtx, tmp_db_mtx;
			db::data *tmp = db::create(lu->source_db, &tmp_db_mtx);

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

				for (int i=0; i<parse; i++)
				{
					if (parsebuf[i] == 0xD || parsebuf[i] == 0xA)
					{
						parsebuf[i] = 0;
						if (i > 0) {
							lines.push(parsebuf);
						}

						const int count = i+1;
						for (int j=0; j<(parse-count); j++)
							parsebuf[j] = parsebuf[j+count];

						parse -= count;
						i = -1;
						continue;
					}
				}

				while (!lines.empty())
				{
					std::string cmd = lines.front();
					std::string argstring;

					std::vector<std::string> args;
					int del = cmd.find_first_of(' ');
					if (del != std::string::npos)
					{
						argstring = cmd.substr(del+1, cmd.size() - del);
						cmd = cmd.substr(0, del);
					}

					while (!argstring.empty())
					{
						del = argstring.find_first_of(' ');
						if (del == std::string::npos)
						{
							args.push_back(argstring);
							break;
						}
						
						args.push_back(argstring.substr(0, del));
						argstring.erase(0, del + 1);
					}

					if (cmd != "poll")
					{
						std::cout << "client [" << cmd << "]";
						if (args.size() > 0)
							std::cout << " arg0=[" << args[0] << "]";
						if (args.size() > 1)
							std::cout << " arg1=[" << args[1] << "]";
						if (args.size() > 2)
							std::cout << " arg2=[" << args[2] << "]";
						std::cout << std::endl;
					}

					if (cmd == "init")
					{
						// see what runtime it is.
						for (int i=0;; i++)
						{
							runtime::descptr p = runtime::get(i);
							if (!p) {
								break;
							}
							if (!strcmp(args[0].c_str(), runtime::desc_str(p))) {
								rt = p;
							}
						}
						
						if (args.size() > 1)
							config = args[1];

						if (!builder)
						{
							builder = builder::create(rt, sourcepath, false, config.c_str(), 1);
							if (builder) {
								builder::enable_liveupdate_builds(builder);
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
							const char *orgpath = lu->_assets_updates[accepted_updates++].c_str();

							char main_object[1024];
							if (!db::base_asset_path(orgpath, main_object, sizeof(main_object)))
								strcpy(main_object, orgpath);

							std::cout << "Client gets [" << main_object << "] (original: " << orgpath << " accepted_updates = " << accepted_updates << std::endl;

							build_db::data *bdb = builder::get_build_db(builder);
							build_db::deplist *dl = build_db::deplist_get(bdb, main_object);
							for (int i=-1;; i++)
							{
								const char *path = (i == -1) ? main_object : build_db::deplist_entry(dl, i);
								if (!path) {
									break;
								}

								// main object always -1
								if (i != -1 && !strcmp(path, main_object))
									continue;

								std::string path_str(path);
								if (!already.count(path_str))
								{
									// need to be found in any of the output dbs.
									buildforclient.push_back(path_str);
									already.insert(path_str);
								}
								
								if (!dl)
								{
									std::cout << "deplist_get(build, '" << main_object << "') returned 0!" << std::endl;
									break;
								}
							}
						}

						leave_lock(lu);
					}

					if (cmd == "build" && !args.empty()) {
						buildforclient.push_back(args[0]);
					}

					while (builder && !buildforclient.empty())
					{
						std::string & tobuild = buildforclient.front();
						std::cout << " i want to build " << tobuild << std::endl;

						enter_lock(lu);

						char b[1024];
						if (db::is_aux_path(tobuild.c_str()))
						{
							if (db::base_asset_path(tobuild.c_str(), b, 1024))
							{
								std::cout << "so it will be " << tobuild << std::endl;
								tobuild = b;
							}
						}

						//
						while (true)
						{
							if (db::exists(tmp, tobuild.c_str(), true) || db::exists(lu->source_db, tobuild.c_str(), true))
								break;

							build_db::record *rec = build_db::find(builder::get_build_db(builder), tobuild.c_str());
							if (!rec || !build_db::get_parent(rec))
								break;


							tobuild = build_db::get_parent(rec);
						}


						// load asset into source db if missing.
						if (!db::exists(tmp, tobuild.c_str(), true) && !db::exists(lu->source_db, tobuild.c_str(), true))
						{
							std::cout << "Can't be built!" << std::endl;
							leave_lock(lu);
							buildforclient.erase(buildforclient.begin());
							continue;
						}

						build::resolve_object(lu->source_db, tobuild.c_str());

						db::data *output = db::create(tmp, &output_db_mtx);

						std::cout << "Sending to client [" << tobuild << "]" << std::endl;

						builder::build_source_object(builder, lu->source_db, tmp, output, tobuild.c_str());
						build::post_build_ptr_update(lu->source_db, output);
						build::post_build_ptr_update(tmp, output);

						// should be done with this now.
						leave_lock(lu);

						package::data *pkg = package::create(output);
						package::add(pkg, tobuild.c_str(), true);
						putki::sstream mf;

						const unsigned int sz = 10*1024*1024;
						char *buf = new char[sz];
						long bytes = package::write(pkg, rt, buf, sz, builder::get_build_db(builder), mf);

						std::cout << "Got a package of " << bytes << " bytes for you." << std::endl;

						char pkttype = 1;
						send(skt, &pkttype, 1, 0);
						for (int i=0; i<4; i++)
						{
							char sz = (bytes >> (i*8)) & 0xff;
							send(skt, &sz, 1, 0);
						}
						send(skt, buf, bytes, 0);

						db::free_and_destroy_objs(output);

						delete [] buf;

						buildforclient.erase(buildforclient.begin());
					}

					lines.pop();
				}
			}

			if (builder) {
				builder::free(builder);
			}

			db::free(tmp);
		}

	}
*/

}

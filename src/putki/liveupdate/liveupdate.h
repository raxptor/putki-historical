namespace putki
{
	namespace db { struct data; }

	namespace liveupdate
	{
		struct data;

		data* start_server(db::data *use_this_db);
		int accept(data *which);
		void stop_server(data *which);
		void service_client(data *lu, const char *sourcepath, int socket);
		void send_update(data *lu, const char *path);
	}
}
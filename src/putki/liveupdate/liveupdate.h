namespace putki
{
	namespace db { struct data; }

	namespace liveupdate
	{
		struct data;

		struct live_update_provider_i
		{
			virtual void on_missing_asset(const char *path) = 0;
		};

		data* start_server();
		int accept(data *which);
		void stop_server(data *which);
		void service_client(const char *sourcepath, int socket);

		void send_update(data *lu, db::data *data, const char *path);
	}
}
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

		data* start_server(const char *name);
		void stop_server(data *which);

		void send_update(data *lu, db::data *data, const char *path);
	}
}
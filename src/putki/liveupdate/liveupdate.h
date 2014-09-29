namespace putki
{
	namespace db { struct data; }

	namespace liveupdate
	{
		enum {
			EDITOR_PORT      = 5555,
			CLIENT_PORT	     = 5556
		};
	
		struct data;
		data *create();
		void free(data *);
		void run_server(data *d);
		
		struct ed_client;
		ed_client *create_editor_connection();
		void run_editor_connection(ed_client *conn);
		void release_editor_connection(ed_client *conn);
	
		void editor_on_edited(ed_client *conn, db::data *db, const char *path);
	}
}
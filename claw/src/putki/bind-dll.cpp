
namespace inki
{
	// these are generated by the putki complier.
	void bind_claw_dll();
}

// Bind all types used by the application, if more than one.
void app_bind_putki_types_dll()
{
	inki::bind_claw_dll();
}

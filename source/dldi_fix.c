#include <gba.h>
#include <dldi.h>

const DISC_INTERFACE* discGetInterface(void)
{
	const DISC_INTERFACE* iface = &io_dldi_data->ioInterface;
	if (iface->startup())
		return iface;
	return NULL;
}



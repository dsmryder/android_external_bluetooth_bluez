/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2007  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>

#include <dbus/dbus.h>
#include <hal/libhal.h>

#include "logging.h"
#include "dbus.h"

#include "hal.h"

static LibHalContext *hal_ctx = NULL;

int hal_init(DBusConnection *conn)
{
	hal_ctx = libhal_ctx_new();
	if (!hal_ctx)
		return -ENOMEM;

	conn = init_dbus(NULL, NULL, NULL);

	if (libhal_ctx_set_dbus_connection(hal_ctx, conn) == FALSE) {
		error("Failed to connect HAL via system bus");
		libhal_ctx_free(hal_ctx);
		hal_ctx = NULL;
		return -EIO;
	}

	if (libhal_ctx_init(hal_ctx, NULL) == FALSE) {
		error("Unable to init HAL context");
		libhal_ctx_free(hal_ctx);
		hal_ctx = NULL;
		return -EIO;
	}

	return 0;
}

void hal_cleanup(void)
{
	if (!hal_ctx)
		return;

	libhal_ctx_shutdown(hal_ctx, NULL);

	libhal_ctx_free(hal_ctx);

	hal_ctx = NULL;
}

int hal_add_device(struct hal_device *device)
{
	char udi[128], *dev;
	char *str = "00000000-0000-1000-8000-00805f9b34fb";

	dev = libhal_new_device(hal_ctx, NULL);

	if (libhal_device_add_capability(hal_ctx, dev,
					"bluetooth", NULL) == FALSE) {
		error("Failed to add device capability");
	}

	if (libhal_device_set_property_string(hal_ctx, dev,
				"bluetooth.uuid", str, NULL) == FALSE) {
		error("Failed to add UUID property");
	}

	if (libhal_device_set_property_bool(hal_ctx, dev,
				"bluetooth.is_connected", FALSE, NULL) == FALSE) {
		error("Failed to add connected state property");
	}

	sprintf(udi, "/org/freedesktop/Hal/devices/bluetooth_network_connection_aabbccddeeff");

	if (libhal_remove_device(hal_ctx, udi, NULL) == FALSE) {
		error("Can't remove old HAL device");
	}

	if (libhal_device_commit_to_gdl(hal_ctx, dev, udi, NULL) == FALSE) {
		error("Failed to add new HAL device");
	}

	return 0;
}

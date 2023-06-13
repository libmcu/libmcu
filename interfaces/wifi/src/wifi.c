/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/wifi.h"
#include "libmcu/port/wifi.h"

struct wifi *wifi_create(int id)
{
	return wifi_port_create(id);
}

int wifi_delete(struct wifi *self)
{
	return wifi_port_delete(self);
}

int wifi_connect(struct wifi *self, const struct wifi_conn_param *param)
{
	return wifi_port_connect(self, param);
}

int wifi_disconnect(struct wifi *self)
{
	return wifi_port_disconnect(self);
}

int wifi_scan(struct wifi *self)
{
	return wifi_port_scan(self);
}

int wifi_enable(struct wifi *self)
{
	return wifi_port_enable(self);
}

int wifi_disable(struct wifi *self)
{
	return wifi_port_disable(self);
}

int wifi_get_status(struct wifi *self, struct wifi_iface_info *info)
{
	return wifi_port_get_status(self, info);
}

int wifi_register_event_callback(struct wifi *self,
		const wifi_event_callback_t cb)
{
	return wifi_port_register_event_callback(self, cb);
}

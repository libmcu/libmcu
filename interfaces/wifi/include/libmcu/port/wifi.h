/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_WIFI_PORT_H
#define LIBMCU_WIFI_PORT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/wifi.h"

struct wifi *wifi_port_create(int id);
int wifi_port_delete(struct wifi *self);
int wifi_port_connect(struct wifi *self, const struct wifi_conn_param *param);
int wifi_port_disconnect(struct wifi *self);
int wifi_port_scan(struct wifi *self);
int wifi_port_enable(struct wifi *self);
int wifi_port_disable(struct wifi *self);
int wifi_port_get_status(struct wifi *self, struct wifi_iface_info *info);
int wifi_port_register_event_callback(struct wifi *self,
		const wifi_event_callback_t cb);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_WIFI_PORT_H */

/*
 * SPDX-FileCopyrightText: 2025 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_IP_H
#define LIBMCU_IP_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

struct lm_ipv4 {
	uint8_t addr[4];
};

struct lm_ipv6 {
	uint8_t addr[16];
	uint8_t zone;
};

struct lm_ipv4_info {
	struct lm_ipv4 ip;
	struct lm_ipv4 netmask;
	struct lm_ipv4 gateway;
};

struct lm_ipv6_info {
	struct lm_ipv6 ip;
	struct lm_ipv6 gateway;
	uint8_t prefix_len; /* prefix length in bits */
};

union lm_ip {
	struct lm_ipv4 v4;
	struct lm_ipv6 v6;
};

union lm_ip_info {
	struct lm_ipv4_info v4;
	struct lm_ipv6_info v6;
};

typedef union lm_ip lm_ip_t;
typedef union lm_ip_info lm_ip_info_t;

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_IP_H */

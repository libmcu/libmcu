/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_PKI_H
#define LIBMCU_PKI_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

struct pki_csr_params {
	const char *common_name;
	const char *organization;
	const char *organization_unit;
	const char *country;
	const char *state;
	const char *locality;
	const char *email;
	const char *serial_number;

	const uint8_t *prikey;
	const size_t prikey_len;
};

int pki_generate_prikey(uint8_t *key_buf, size_t key_bufsize);
int pki_generate_keypair(uint8_t *key_buf, size_t key_bufsize,
		uint8_t *pub_buf, size_t pub_bufsize);
int pki_generate_csr(uint8_t *csr_buf, size_t csr_bufsize,
		const struct pki_csr_params *params);
int pki_verify_cert(const uint8_t *cacert, size_t cacert_len,
		const uint8_t *cert, size_t cert_len);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_PKI_H */

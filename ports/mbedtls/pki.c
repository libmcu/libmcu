/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/pki.h"

#include <stdio.h>
#include <string.h>

#include "mbedtls/entropy.h"
#include "mbedtls/hmac_drbg.h"
#include "mbedtls/ecp.h"
#include "mbedtls/pk.h"
#include "mbedtls/x509_csr.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/version.h"

static int generate_ec_prikey(uint8_t *key_buf, size_t key_bufsize)
{
	mbedtls_pk_context pk;
	mbedtls_entropy_context entropy;
	mbedtls_hmac_drbg_context hmac_drbg;
	const char *pers = "ecp_genkey";
	int rc = 0;

	mbedtls_entropy_init(&entropy);
	mbedtls_hmac_drbg_init(&hmac_drbg);
	mbedtls_pk_init(&pk);

	if ((rc = mbedtls_hmac_drbg_seed(&hmac_drbg,
			mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
			mbedtls_entropy_func, &entropy,
			(const unsigned char *)pers, strlen(pers))) != 0) {
		goto out_free;
	}

	if ((rc = mbedtls_pk_setup(&pk,
			mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY))) != 0) {
		goto out_free;
	}

	if ((rc = mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1,
			mbedtls_pk_ec(pk), mbedtls_hmac_drbg_random,
			&hmac_drbg)) != 0) {
		goto out_free;
	}

	if ((rc = mbedtls_pk_write_key_pem(&pk, key_buf, key_bufsize)) != 0) {
		goto out_free;
	}

out_free:
	mbedtls_pk_free(&pk);
	mbedtls_hmac_drbg_free(&hmac_drbg);
	mbedtls_entropy_free(&entropy);

	return rc;
}

static int get_pubkey_from_prikey(uint8_t *pub_buf, size_t pub_bufsize,
		const uint8_t *prikey, const size_t prikey_len)
{
	int err = 0;
	mbedtls_pk_context pk;

	mbedtls_pk_init(&pk);

#if MBEDTLS_VERSION_MAJOR < 3
	err = mbedtls_pk_parse_key(&pk, prikey, prikey_len, NULL, 0);
#else
	mbedtls_hmac_drbg_context hmac_drbg;
	err = mbedtls_pk_parse_key(&pk, prikey, prikey_len, NULL, 0,
			mbedtls_hmac_drbg_random, &hmac_drbg);
#endif
	if (err || (err = mbedtls_pk_write_pubkey_pem(&pk,
			pub_buf, pub_bufsize))) {
		goto out_cleanup;
	}

out_cleanup:
	mbedtls_pk_free(&pk);

	return err;
}

static int set_subject_str(char *subject_str, size_t subject_str_len,
		const struct pki_csr_params *params)
{
	int len = 0;

	memset(subject_str, 0, subject_str_len);

	if (params->common_name) {
		int rc = snprintf(subject_str, subject_str_len, "CN=%s",
				params->common_name);
		len += rc > 0? rc : 0;
	}
	if (params->organization) {
		int rc = snprintf(subject_str + len, subject_str_len - len,
				"%sO=%s", len? ",":"", params->organization);
		len += rc > 0? rc : 0;
	}
	if (params->organization_unit) {
		int rc = snprintf(subject_str + len, subject_str_len - len,
				"%sOU=%s", len? ",":"", params->organization_unit);
		len += rc > 0? rc : 0;
	}
	if (params->country) {
		int rc = snprintf(subject_str + len, subject_str_len - len,
				"%sC=%s", len? ",":"", params->country);
		len += rc > 0? rc : 0;
	}
	if (params->state) {
		int rc = snprintf(subject_str + len, subject_str_len - len,
				"%sST=%s", len? ",":"", params->state);
		len += rc > 0? rc : 0;
	}
	if (params->locality) {
		int rc = snprintf(subject_str + len, subject_str_len - len,
				"%sL=%s", len? ",":"", params->locality);
		len += rc > 0? rc : 0;
	}
	if (params->email) {
		int rc = snprintf(subject_str + len, subject_str_len - len,
				"%sR=%s", len? ",":"", params->email);
		len += rc > 0? rc : 0;
	}
	if (params->serial_number) {
		int rc = snprintf(subject_str + len, subject_str_len - len,
				"%sserialNumber=%s", len? ",":"",
				params->serial_number);
		len += rc > 0? rc : 0;
	}

	return len;
}

int pki_generate_prikey(uint8_t *key_buf, size_t key_bufsize)
{
	return generate_ec_prikey(key_buf, key_bufsize);
}

int pki_generate_keypair(uint8_t *key_buf, size_t key_bufsize,
		uint8_t *pub_buf, size_t pub_bufsize)
{
	int err = generate_ec_prikey(key_buf, key_bufsize);

	if (!err) {
		err = get_pubkey_from_prikey(pub_buf, pub_bufsize,
				key_buf, key_bufsize);
	}

	return err;
}

int pki_generate_csr(uint8_t *csr_buf, size_t csr_bufsize,
		const uint8_t *prikey, size_t prikey_len,
		const struct pki_csr_params *params)
{
	mbedtls_pk_context pk;
	mbedtls_entropy_context entropy;
	mbedtls_hmac_drbg_context hmac_drbg;
	const char *pers = "ecp_genkey";
	mbedtls_x509write_csr req;
	int rc = 0;

	mbedtls_entropy_init(&entropy);
	mbedtls_hmac_drbg_init(&hmac_drbg);
	mbedtls_pk_init(&pk);
	mbedtls_x509write_csr_init(&req);

	if ((rc = mbedtls_hmac_drbg_seed(&hmac_drbg,
			mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
			mbedtls_entropy_func, &entropy,
			(const unsigned char *)pers, strlen(pers))) != 0) {
		goto out_free;
	}

#if MBEDTLS_VERSION_MAJOR < 3
	rc = mbedtls_pk_parse_key(&pk, prikey, prikey_len + 1, NULL, 0);
#else
	rc = mbedtls_pk_parse_key(&pk, prikey, prikey_len + 1, NULL, 0,
			mbedtls_hmac_drbg_random, &hmac_drbg);
#endif
	if (rc != 0) {
		goto out_free;
	}

#define SUBJECT_STR_MAXLEN	128
	char subject_str[SUBJECT_STR_MAXLEN];
	set_subject_str(subject_str, SUBJECT_STR_MAXLEN, params);

	mbedtls_x509write_csr_set_subject_name(&req, subject_str);
	mbedtls_x509write_csr_set_key(&req, &pk);
	mbedtls_x509write_csr_set_md_alg(&req, MBEDTLS_MD_SHA256);

	if ((rc = mbedtls_x509write_csr_pem(&req, csr_buf, csr_bufsize,
			mbedtls_hmac_drbg_random, &hmac_drbg)) != 0) {
		goto out_free;
	}

out_free:
	mbedtls_x509write_csr_free(&req);
	mbedtls_pk_free(&pk);
	mbedtls_hmac_drbg_free(&hmac_drbg);
	mbedtls_entropy_free(&entropy);

	return rc;
}

int pki_verify_cert(const uint8_t *cacert, size_t cacert_len,
		const uint8_t *cert, size_t cert_len)
{
	mbedtls_x509_crt x509_ca;
	mbedtls_x509_crt x509_device;
	uint32_t flags;
	int err;

	mbedtls_x509_crt_init(&x509_ca);
	mbedtls_x509_crt_init(&x509_device);

	if ((err = mbedtls_x509_crt_parse(&x509_ca, cacert, cacert_len + 1))) {
		goto out_free;
	}

	if ((err = mbedtls_x509_crt_parse(&x509_device, cert, cert_len + 1))) {
		goto out_free;
	}

	if ((err = mbedtls_x509_crt_verify(&x509_device, &x509_ca, NULL, NULL,
			&flags, NULL, NULL))) {
		goto out_free;
	}

out_free:
	mbedtls_x509_crt_free(&x509_device);
	mbedtls_x509_crt_free(&x509_ca);

	return err;
}

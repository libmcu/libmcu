/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/pki.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/rand.h>

static int generate_ec_prikey(uint8_t *key_buf, size_t key_bufsize)
{
	EVP_PKEY_CTX *pctx = NULL;
	EVP_PKEY *pkey = NULL;
	BIO *bio = NULL;
	int err = 0;

	if (!(pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL))) {
		goto cleanup;
	}

	if (EVP_PKEY_keygen_init(pctx) <= 0 ||
			EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx,
					NID_X9_62_prime256v1) <= 0 ||
			EVP_PKEY_keygen(pctx, &pkey) <= 0) {
		goto cleanup;
	}

	if (!(bio = BIO_new(BIO_s_mem()))) {
		goto cleanup;
	}

	if (!PEM_write_bio_PrivateKey(bio, pkey, NULL, NULL, 0, NULL, NULL)) {
		goto cleanup;
	}

	if ((err = BIO_read(bio, key_buf, (int)key_bufsize)) > 0) {
		err = 0;
	}

cleanup:
	if (err != 0) {
		ERR_print_errors_fp(stderr);
	}
	BIO_free(bio);
	EVP_PKEY_free(pkey);
	EVP_PKEY_CTX_free(pctx);

	return err;
}

static int get_pubkey_from_prikey(uint8_t *pub_buf, size_t pub_bufsize,
		const uint8_t *prikey, const size_t prikey_len)
{
	BIO *bio = BIO_new_mem_buf(prikey, (int)prikey_len);
	if (!bio) {
		return -ENOMEM;
	}

	EVP_PKEY *pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
	BIO *pub_bio = BIO_new(BIO_s_mem());
	int err = 0;

	if (!pkey || !pub_bio) {
		goto cleanup;
	}

	if (!PEM_write_bio_PUBKEY(pub_bio, pkey)) {
		goto cleanup;
	}

	if ((err = BIO_read(pub_bio, pub_buf, (int)pub_bufsize)) > 0) {
		err = 0;
	}

cleanup:
	if (err != 0) {
		ERR_print_errors_fp(stderr);
	}
	BIO_free(pub_bio);
	EVP_PKEY_free(pkey);
	BIO_free(bio);

	return err;
}

int pki_generate_prikey(uint8_t *key_buf, size_t key_bufsize)
{
	return generate_ec_prikey(key_buf, key_bufsize);
}

int pki_generate_keypair(uint8_t *key_buf, size_t key_bufsize,
		uint8_t *pub_buf, size_t pub_bufsize)
{
	if (generate_ec_prikey(key_buf, key_bufsize) != 0) {
		return -EIO;
	}
	return get_pubkey_from_prikey(pub_buf, pub_bufsize,
			key_buf, key_bufsize);
}

int pki_generate_csr(uint8_t *csr_buf, size_t csr_bufsize,
		const uint8_t *prikey, size_t prikey_len,
		const struct pki_csr_params *params)
{
	BIO *bio = BIO_new_mem_buf(prikey, (int)prikey_len);
	if (!bio) {
		return -ENOMEM;
	}

	EVP_PKEY *pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
	X509_REQ *req = X509_REQ_new();
	X509_NAME *name = X509_NAME_new();
	BIO *csr_mem = BIO_new(BIO_s_mem());
	int err = 0;

	if (!pkey || !req || !csr_mem || !name) {
		goto cleanup;
	}

	if (params->common_name) {
		X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
				(const unsigned char *)
				params->common_name, -1, -1, 0);
	}
	if (params->organization) {
		X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC,
				(const unsigned char *)
				params->organization, -1, -1, 0);
	}
	if (params->organization_unit) {
		X509_NAME_add_entry_by_txt(name, "OU", MBSTRING_ASC,
				(const unsigned char *)
				params->organization_unit, -1, -1, 0);
	}
	if (params->country) {
		X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC,
				(const unsigned char *)
				params->country, -1, -1, 0);
	}
	if (params->state) {
		X509_NAME_add_entry_by_txt(name, "ST", MBSTRING_ASC,
				(const unsigned char *)
				params->state, -1, -1, 0);
	}
	if (params->locality) {
		X509_NAME_add_entry_by_txt(name, "L", MBSTRING_ASC,
				(const unsigned char *)
				params->locality, -1, -1, 0);
	}
	if (params->email) {
		X509_NAME_add_entry_by_txt(name, "emailAddress", MBSTRING_ASC,
				(const unsigned char *)
				params->email, -1, -1, 0);
	}

	X509_REQ_set_subject_name(req, name);
	X509_REQ_set_pubkey(req, pkey);

	if (!X509_REQ_sign(req, pkey, EVP_sha256())) goto cleanup;
	if (!PEM_write_bio_X509_REQ(csr_mem, req)) goto cleanup;

	err = BIO_read(csr_mem, csr_buf, (int)csr_bufsize);

cleanup:
	if (err != 0) {
		ERR_print_errors_fp(stderr);
	}
	BIO_free(csr_mem);
	X509_NAME_free(name);
	X509_REQ_free(req);
	EVP_PKEY_free(pkey);
	BIO_free(bio);

	return err;
}

int pki_verify_cert(const uint8_t *cacert, size_t cacert_len,
		const uint8_t *cert, size_t cert_len)
{
	BIO *bio_ca = BIO_new_mem_buf(cacert, (int)cacert_len);
	BIO *bio_cert = BIO_new_mem_buf(cert, (int)cert_len);
	X509 *ca = PEM_read_bio_X509(bio_ca, NULL, NULL, NULL);
	X509 *crt = PEM_read_bio_X509(bio_cert, NULL, NULL, NULL);
	X509_STORE *store = X509_STORE_new();
	X509_STORE_CTX *ctx = X509_STORE_CTX_new();
	int err = 0;

	if (!ca || !crt || !store || !ctx) {
		goto cleanup;
	}
	if (!X509_STORE_add_cert(store, ca) ||
			!X509_STORE_CTX_init(ctx, store, crt, NULL)) {
		goto cleanup;
	}

	if ((err = X509_verify_cert(ctx)) == 1) {
		err = 0;
	}

cleanup:
	if (err != 0) {
		ERR_print_errors_fp(stderr);
	}
	X509_STORE_CTX_free(ctx);
	X509_STORE_free(store);
	X509_free(ca);
	X509_free(crt);
	BIO_free(bio_ca);
	BIO_free(bio_cert);

	return err;
}

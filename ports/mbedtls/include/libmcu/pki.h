/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
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
};

/**
 * @brief Generates a private key.
 *
 * This function generates a private key and stores it in the provided buffer.
 *
 * @param[out] key_buf Pointer to the buffer where the generated private key
 *                     will be stored.
 * @param[in] key_bufsize Size of the private key buffer.
 *
 * @return 0 if the private key is generated successfully, non-zero otherwise.
 */
int pki_generate_prikey(uint8_t *key_buf, size_t key_bufsize);

/**
 * @brief Generates a public-private key pair.
 *
 * This function generates a public-private key pair and stores them in the
 * provided buffers. The private key is stored in `key_buf` and the public key
 * is stored in `pub_buf`.
 *
 * @param[out] key_buf Pointer to the buffer where the generated private key
 *                     will be stored.
 * @param[in] key_bufsize Size of the private key buffer.
 * @param[out] pub_buf Pointer to the buffer where the generated public key will
 *                     be stored.
 * @param[in] pub_bufsize Size of the public key buffer.
 *
 * @return 0 if the key pair is generated successfully, non-zero otherwise.
 */
int pki_generate_keypair(uint8_t *key_buf, size_t key_bufsize,
                         uint8_t *pub_buf, size_t pub_bufsize);

/**
 * @brief Generates a Certificate Signing Request (CSR).
 *
 * This function generates a CSR based on the provided private key and
 * parameters. The generated CSR is stored in the provided buffer.
 *
 * @param[out] csr_buf Pointer to the buffer where the generated CSR will be
 *                     stored.
 * @param[out] csr_bufsize Size of the CSR buffer.
 * @param[in] prikey Pointer to the private key used for generating the CSR.
 * @param[in] prikey_len Length of the private key.
 * @param[in] params Pointer to the structure containing the parameters for the
 *                   CSR.
 *
 * @return 0 if the CSR is generated successfully, non-zero otherwise.
 */
int pki_generate_csr(uint8_t *csr_buf, size_t csr_bufsize,
                     const uint8_t *prikey, size_t prikey_len,
                     const struct pki_csr_params *params);

/**
 * @brief Verifies a certificate against a given CA certificate.
 *
 * This function verifies the authenticity of a certificate by checking it
 * against a given Certificate Authority (CA) certificate.
 *
 * @param[in] cacert Pointer to the CA certificate against which the certificate
 *               will be verified.
 * @param[in] cacert_len Length of the CA certificate.
 * @param[in] cert Pointer to the certificate to be verified.
 * @param[in] cert_len Length of the certificate to be verified.
 *
 * @return 0 if the certificate is verified successfully, non-zero otherwise.
 */
int pki_verify_cert(const uint8_t *cacert, size_t cacert_len,
                    const uint8_t *cert, size_t cert_len);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_PKI_H */

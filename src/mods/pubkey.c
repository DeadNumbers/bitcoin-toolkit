#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gmp.h>
#include "pubkey.h"
#include "privkey.h"
#include "point.h"
#include "crypto.h"
#include "base58check.h"
#include "bech32.h"
#include "hex.h"
#include "network.h"
#include "mem.h"
#include "assert.h"

#define ADDRESS_VERSION_BIT_MAINNET   0x00
#define ADDRESS_VERSION_BIT_TESTNET   0x6F
#define PUBKEY_COMPRESSED_FLAG_EVEN   0x02
#define PUBKEY_COMPRESSED_FLAG_ODD    0x03
#define PUBKEY_UNCOMPRESSED_FLAG      0x04
#define PUBKEY_POINTS                 (PRIVKEY_LENGTH * 8)

struct PubKey{
	unsigned char data[PUBKEY_UNCOMPRESSED_LENGTH + 1];
};

PubKey pubkey_get(PrivKey k) {
	size_t i, l;
	char *prvkeyhex;
	mpz_t prvkey;
	Point pubkey;
	Point points[PUBKEY_POINTS];
	PubKey r;
	
	assert(k);
	
	NEW0(r);

	// Load private key from hex string, truncating the compression flag.
	mpz_init(prvkey);
	prvkeyhex = privkey_to_hex(k);
	prvkeyhex[PRIVKEY_LENGTH * 2] = '\0';
	mpz_set_str(prvkey, prvkeyhex, 16);
	free(prvkeyhex);
	
	// Initalize the points
	point_init(&pubkey);
	for (i = 0; i < PUBKEY_POINTS; ++i) {
		point_init(points + i);
	}

	// Calculating public key
	point_set_generator(&points[0]);
	for (i = 1; i < PUBKEY_POINTS; ++i) {
		point_double(&points[i], points[i-1]);
		assert(point_verify(points[i]));
	}

	// Add all points corresponding to 1 bits
	for (i = 0; i < PUBKEY_POINTS; ++i) {
		if (mpz_tstbit(prvkey, i) == 1) {
			if (mpz_cmp_ui(pubkey.x, 0) == 0 && mpz_cmp_ui(pubkey.y, 0) == 0) {
				point_set(&pubkey, points[i]);
			} else {
				point_add(&pubkey, pubkey, points[i]);
			}
			assert(point_verify(pubkey));
		}
	}
	
	// Setting compression flag
	if (privkey_is_compressed(k)) {
		if (mpz_even_p(pubkey.y)) {
			r->data[0] = PUBKEY_COMPRESSED_FLAG_EVEN;
		} else {
			r->data[0] = PUBKEY_COMPRESSED_FLAG_ODD;
		}
	} else {
		r->data[0] = PUBKEY_UNCOMPRESSED_FLAG;
	}
	
	// Exporting x,y coordinates as byte string, making sure to leave leading
	// zeros if either exports as less than 32 bytes.
	l = (mpz_sizeinbase(pubkey.x, 2) + 7) / 8;
	mpz_export(r->data + 1 + (32 - l), &i, 1, 1, 1, 0, pubkey.x);
	assert(l == i);
	if (!privkey_is_compressed(k)) {
		l = (mpz_sizeinbase(pubkey.y, 2) + 7) / 8;
		mpz_export(r->data + 33 + (32 - l), &i, 1, 1, 1, 0, pubkey.y);
		assert(l == i);
	}

	// Clear all points
	mpz_clear(prvkey);
	for (i = 0; i < PUBKEY_POINTS; ++i) {
		point_clear(points + i);
	}
	
	return r;
}

PubKey pubkey_compress(PubKey k) {
	mpz_t y;
	size_t point_length = 32;
	
	assert(k);
	
	if (k->data[0] == PUBKEY_COMPRESSED_FLAG_EVEN || k->data[0] == PUBKEY_COMPRESSED_FLAG_ODD) {
		return k;
	}

	mpz_init(y);

	mpz_import(y, point_length, 1, 1, 1, 0, k->data + 1 + point_length);
	
	if (mpz_even_p(y)) {
		k->data[0] = PUBKEY_COMPRESSED_FLAG_EVEN;
	} else {
		k->data[0] = PUBKEY_COMPRESSED_FLAG_ODD;
	}

	mpz_clear(y);
	
	return k;
}

int pubkey_is_compressed(PubKey k) {
	assert(k);
	if (k->data[0] == PUBKEY_COMPRESSED_FLAG_EVEN || k->data[0] == PUBKEY_COMPRESSED_FLAG_ODD) {
		return 1;
	}
	return 0;
}

char *pubkey_to_hex(PubKey k) {
	int i, l;
	char *r;
	
	assert(k);
	
	if (k->data[0] == PUBKEY_UNCOMPRESSED_FLAG) {
		l = (PUBKEY_UNCOMPRESSED_LENGTH * 2) + 2;
	} else if (k->data[0] == PUBKEY_COMPRESSED_FLAG_EVEN || k->data[0] == PUBKEY_COMPRESSED_FLAG_ODD) {
		l = (PUBKEY_COMPRESSED_LENGTH * 2) + 2;
	} else {
		return NULL;
	}

	r = ALLOC(l + 1);
	if (r == NULL) {
		return NULL;
	}

	memset(r, 0, l + 1);
	
	for (i = 0; i < l/2; ++i) {
		sprintf(r + (i * 2), "%02x", k->data[i]);
	}
	
	return r;
}

unsigned char *pubkey_to_raw(PubKey k, size_t *rl) {
	int i, l;
	char *s;
	unsigned char *r;
	
	s = pubkey_to_hex(k);
	
	l = strlen(s);
	
	r = ALLOC(l/2);
	
	for (i = 0; i < l; i += 2) {
		r[i/2] = hex_to_dec(s[i], s[i+1]);
	}
	
	*rl = l/2;
	
	return r;
}

char *pubkey_to_address(PubKey k) {
	size_t l;
	unsigned char *sha, *rmd;
	unsigned char r[21];

	if (pubkey_is_compressed(k)) {
		l = PUBKEY_COMPRESSED_LENGTH + 1;
	} else {
		l = PUBKEY_UNCOMPRESSED_LENGTH + 1;
	}

	// RMD(SHA(data))
	sha = crypto_get_sha256(k->data, l);
	rmd = crypto_get_rmd160(sha, 32);

	// Set address version bit
	if (network_is_main()) {
		r[0] = ADDRESS_VERSION_BIT_MAINNET;
	} else if (network_is_test()) {
		r[0] = ADDRESS_VERSION_BIT_TESTNET;
	}
	
	// Append rmd data
	memcpy(r + 1, rmd, 20);
	
	// Free resources
	FREE(sha);
	FREE(rmd);
	
	return base58check_encode(r, 21);
}

char *pubkey_to_bech32address(PubKey k) {
	size_t l;
	unsigned char *sha, *rmd;
	char *output;

	if (pubkey_is_compressed(k)) {
		l = PUBKEY_COMPRESSED_LENGTH + 1;
	} else {
		l = PUBKEY_UNCOMPRESSED_LENGTH + 1;
	}

	// RMD(SHA(data))
	sha = crypto_get_sha256(k->data, l);
	rmd = crypto_get_rmd160(sha, 32);

	output = ALLOC(100);
	bech32_get_address(output, rmd, 20);
	
	// Free resources
	FREE(sha);
	FREE(rmd);

	return output;
}

void pubkey_free(PubKey k) {
	FREE(k);
}


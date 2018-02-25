/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the MIT License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <gmp.h>
#include "mods/privkey.h"
#include "mods/base58.h"
#include "mods/crypto.h"
#include "mods/hex.h"
#include "mods/mem.h"
#include "mods/assert.h"

#define BUFFER_SIZE             1000
#define INPUT_NEW               1
#define INPUT_WIF               2
#define INPUT_HEX               3
#define INPUT_RAW               4
#define INPUT_STR               5
#define INPUT_DEC               6
#define OUTPUT_WIF              1
#define OUTPUT_HEX              2
#define OUTPUT_RAW              3
#define OUTPUT_COMPRESS         1
#define OUTPUT_UNCOMPRESS       2
#define TRUE                    1
#define FALSE                   0

/*
 * Static Variable Declarations
 */
static unsigned char input_buffer[BUFFER_SIZE];

int btk_privkey_main(int argc, char *argv[]) {
	int o;
	size_t i, c;
	mpz_t d;
	PrivKey key = NULL;
	unsigned char *t;
	
	// Default flags
	int input_format       = INPUT_NEW;
	int output_format      = OUTPUT_WIF;
	int output_compression = FALSE;
	int output_newline     = FALSE;

	// Zero the input buffer
	memset(input_buffer, 0, BUFFER_SIZE);
	
	// Process arguments
	while ((o = getopt(argc, argv, "nwhrsdWHRCUN")) != -1) {
		switch (o) {
			// Input format
			case 'n':
				input_format = INPUT_NEW;
				output_compression = OUTPUT_COMPRESS;
				break;
			case 'w':
				input_format = INPUT_WIF;
				break;
			case 'h':
				input_format = INPUT_HEX;
				break;
			case 'r':
				input_format = INPUT_RAW;
				break;
			case 's':
				input_format = INPUT_STR;
				output_compression = OUTPUT_COMPRESS;
				break;
			case 'd':
				input_format = INPUT_DEC;
				output_compression = OUTPUT_COMPRESS;
				break;

			// Output format
			case 'W':
				output_format = OUTPUT_WIF;
				break;
			case 'H':
				output_format = OUTPUT_HEX;
				break;
			case 'R':
				output_format = OUTPUT_RAW;
				break;
			
			// Output Compression
			case 'C':
				output_compression = OUTPUT_COMPRESS;
				break;
			case 'U':
				output_compression = OUTPUT_UNCOMPRESS;
				break;

			// Other options
			case 'N':
				output_newline = TRUE;
				break;

			// Unknown option
			case '?':
				if (isprint(optopt))
					fprintf (stderr, "Unknown option '-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character '\\x%x'.\n", optopt);
				return EXIT_FAILURE;
		}
	}
	
	// Process Input
	switch (input_format) {
		case INPUT_NEW:
			key = privkey_new();
			break;
		case INPUT_WIF:
			c = read(STDIN_FILENO, input_buffer, BUFFER_SIZE - 1);
			for (i = 0; i < c; ++i)
				if (!base58_ischar(input_buffer[i]))
					break;
			if (i < PRIVKEY_WIF_LENGTH_MIN) {
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}
			input_buffer[c] = '\0';
			key = privkey_from_wif((char *)input_buffer);
			break;
		case INPUT_HEX:
			c = read(STDIN_FILENO, input_buffer, BUFFER_SIZE - 1);

			// Ignore white spaces at the end of input
			while (isspace((int)input_buffer[c-1]))
				--c;

			if (c < PRIVKEY_LENGTH * 2) {
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}
			if (c == PRIVKEY_LENGTH * 2 + 1) {           // Incomplete compression flag
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}
			for (i = 0; i < c; ++i) {
				if (!hex_ischar(input_buffer[i])) {
					fprintf(stderr, "Error: Invalid input.\n");
					return EXIT_FAILURE;
				}
			}
			input_buffer[c] = '\0';
			key = privkey_from_hex((char *)input_buffer);
			break;
		case INPUT_RAW:
			c = read(STDIN_FILENO, input_buffer, BUFFER_SIZE - 1);
			if (c < PRIVKEY_LENGTH) {
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}
			key = privkey_from_raw(input_buffer, c);
			break;
		case INPUT_STR:
			c = read(STDIN_FILENO, input_buffer, BUFFER_SIZE - 1);
			t = crypto_get_sha256(input_buffer, c);
			key = privkey_from_raw(t, 32);
			free(t);
			break;
		case INPUT_DEC:
			c = read(STDIN_FILENO, input_buffer, BUFFER_SIZE - 1);
			for (i = 0; i < c; ++i) {
				if (input_buffer[i] < '0' || input_buffer[i] > '9') {
					fprintf(stderr, "Error: Invalid input.\n");
					return EXIT_FAILURE;
				}
			}
			mpz_init(d);
			mpz_set_str(d, (char *)input_buffer, 10);
			i = (mpz_sizeinbase(d, 2) + 7) / 8;
			if (i > PRIVKEY_LENGTH)
				i = PRIVKEY_LENGTH;
			memset(input_buffer, 0, BUFFER_SIZE);
			mpz_export(input_buffer + PRIVKEY_LENGTH - i, &c, 1, 1, 1, 0, d);
			mpz_clear(d);
			key = privkey_from_raw(input_buffer, PRIVKEY_LENGTH);
			break;
	}
	
	// Make sure we have a key
	assert(key);
	
	// Set output compression only if the option is set. Otherwise,
	// compression is based on input.
	switch (output_compression) {
		case FALSE:
			break;
		case OUTPUT_COMPRESS:
			key = privkey_compress(key);
			break;
		case OUTPUT_UNCOMPRESS:
			key = privkey_uncompress(key);
			break;
	}
	
	// Write output
	switch (output_format) {
		case OUTPUT_WIF:
			printf("%s", privkey_to_wif(key));
			break;
		case OUTPUT_HEX:
			printf("%s", privkey_to_hex(key));
			break;
		case OUTPUT_RAW:
			t = privkey_to_raw(key, &c);
			for (i = 0; i < c; ++i) {
				printf("%c", t[i]);
			}
			free(t);
			break;
	}
	// Process format flags
	switch (output_newline) {
		case TRUE:
			printf("\n");
			break;
	}

	// Free key
	privkey_free(key);

	// Return
	return EXIT_SUCCESS;
}

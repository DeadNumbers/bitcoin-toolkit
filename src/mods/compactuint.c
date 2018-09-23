#include <stdlib.h>
#include <stdint.h>
#include "assert.h"

int compactuint_get_value(uint64_t *output, unsigned char *input, size_t input_len)
{
	int i, j, r;
	
	assert(output);
	assert(input);
	assert(input_len);

	r = 0;

	if (*input <= 0xfc)
	{
		*output = *input;
		r = 1;
	}
	else
	{
		if (*input == 0xfd)
		{
			j = 2;
		}
		else if (*input == 0xfe)
		{
			j = 4;
		}
		else if (*input == 0xff)
		{
			j = 8;
		}

		r = j + 1;

		if (input_len < (size_t)r)
		{
			return -1;
		}

		*output = 0;
		++input;

		for (i = 0; i < j; ++i, ++input)
		{
			*output <<= 8;
			*output += *input;
		}
	}
	
	return r;
}


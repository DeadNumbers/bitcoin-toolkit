/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "mods/config.h"

int btk_version_init(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	return 1;
}

int btk_version_main(void)
{
	printf("Bitcoin Toolkit Version %d.%d.%d\n", BTK_VERSION_MAJOR, BTK_VERSION_MINOR, BTK_VERSION_REVISION);

	return 1;
}

int btk_version_cleanup(void)
{
	return 1;
}
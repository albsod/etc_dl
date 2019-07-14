#include "clopt.h"
#include <stdio.h>
#include <stdlib.h>

/* Function definitions */
void help_option(char *argv)
{
	printf("usage: %s [OPTION]\n", argv);
	printf("  -d        get the URL of a different date relative to today's,\n");
	printf("            e.g. -d -1 for yesterday or -d 1 for tomorrow\n");
	printf("  -h        print this usage and exit\n");
	printf("  -o        open the file after download. May be combined\n");
	printf("            with -u to open the remote URL\n");
	printf("  -t        target directory (instead of the current directory)\n");
	printf("  -u        print the URL; don't download using libcurl.\n");
	printf("            May be combined with -o to open the remote URL\n");
	printf("  -v        print version information and exit\n");
	exit(0);
}

void version_option(void)
{
	printf("ETC-dl 1.0\n");
	printf("Copyright © 2018-2019 Albin Söderqvist\n");
	printf("License: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>\n");
	printf("This is free software, and you are welcome to redistribute it.\n");
	printf("There is NO WARRANTY.  See COPYING for details.\n");
	exit(0);
}

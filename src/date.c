#include "date.h"
#include <stdio.h>
#include <stdlib.h>

/* Function definitions */
/************************/

/* get POSIX time for a past date*/
time_t parse_date(const char *str)
{

	/* struct tm ti={0}; */
	/* With the initialization below clang doesn't complain; it does
	   with the one above */
	struct tm ti = {
		.tm_sec = 0,
		.tm_min = 0,
		.tm_hour = 0,
		.tm_mday = 0,
		.tm_mon = 0,
		.tm_year = 0,
		.tm_wday = 0,
		.tm_isdst = 0,
	};
		
	if (sscanf(str, "%d-%d-%d", &ti.tm_year, &ti.tm_mon, &ti.tm_mday) !=3) {
		fprintf(stderr, "Failed to parse date.\n");
		exit(1);
	}
	ti.tm_year-=1900;
	ti.tm_mon-=1;
	return mktime(&ti);
}

/* add days to a date */
void date_plus_days(struct tm *date, long int days)
{
	const time_t ONE_DAY = 93600; /* (24 * 60 * 60) */

	/* Seconds since start of epoch */
	time_t date_seconds = mktime(date) + (days * ONE_DAY);

	/* update caller's date */	
	/* Use localtime because mktime converts to UTC */
	*date = *localtime(&date_seconds);
}

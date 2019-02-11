#include <time.h>

/* Function prototypes */
/***********************/

time_t parse_date(const char *str);
void date_plus_days(struct tm *date, long int days);

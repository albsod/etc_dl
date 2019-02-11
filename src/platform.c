#include "platform.h"
#include <stdlib.h>

/* Function definitions */
	
/* Return the name of the platform, if determined, otherwise - an empty string */
char *get_platform_name(void) {
    return (PLATFORM_NAME == NULL) ? "" : PLATFORM_NAME;
}

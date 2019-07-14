/***************************************************************************
 *
 * ETC-dl: Download Dagens ETC
 * Copyright (c) 2018-2019  Albin Söderqvist <albin@fripost.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ETC-dl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * This file consists of example code from the curl project
 * Copyright (c) 1996-2019  Daniel Stenberg <daniel@haxx.se> et al.
 * See the copyright and permission notice included with this program.
 *
 **************************************************************************/

#include <string.h>
#include "curly.h"

/* Function definitions */

size_t write_data(void *ptr, size_t size,
			 size_t nmemb, void *stream)
{
	size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
	return written;
}

size_t header_callback(char *buffer, size_t size,
			      size_t nitems)
{
	/* received header is nitems * size long in 'buffer' NOT ZERO TERMINATED */

	/* If content type is found but not "application/pdf",
	   return -1, which stops download */
	if ((strstr(buffer, "Content-Type") != NULL) &&
	     (strstr(buffer, "application/pdf") == NULL))  {
		return -1;
	}
		
	/* The pointer named userdata is the one you set with the CURLOPT_HEADERDATA option. */
	return nitems * size;
}

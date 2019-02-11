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
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
 * GNU General Public License for more details.			  
 * 								
 * You should have received a copy of the GNU General Public License    
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * This program makes use of the curl library.
 * Copyright © 1996-2019  Daniel Stenberg <daniel@haxx.se>, et al.
 * See the Copyright and permission notice in src/libcurl/COPYING.
 *
 **************************************************************************/

#include <curl/curl.h>

/* Variables */
CURL *curl_handle;

/* Function prototypes */

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
size_t header_callback(char *buffer, size_t size, size_t nitems);


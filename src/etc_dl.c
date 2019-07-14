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
 * The download function in this file consists of code from the curl project
 * Copyright (c) 1996-2019  Daniel Stenberg <daniel@haxx.se> et al.
 * See the copyright and permission notice included with this program.
 *
 **************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "clopt.h"
#include "platform.h"
#include "curly.h"
#include "date.h"
#define	PATH_SIZE 1024

static char *get_url(const long mod_date, const char *known_date,
		     const long known_issue_long);
static char *get_path(const int target, const char *target_dir,
		      const char *etc_url);
static int download(const char *path, const char *etc_url);
static void open_file(char *path);
static int decrease_month(char *etc_url);
static void update_config(FILE *conf, char *config_path, char *etc_url);

int main(int argc, char *argv[])
{
	/* Variables */
	int opt, printurl = 0, openpdf = 0, target = 0, errflg = 0;
	long mod_date = 0;
	char *target_dir = NULL;
	struct stat st = {0};


	while ((opt = getopt(argc, argv, ":hvuod:t:")) != -1) {
		switch(opt) {
		case 'h':
			help_option(argv[0]);
			break;
		case 'v':
			version_option();
			break;
		case 'u':
			printurl = 1;
			break;
		case 't':
			if (stat(optarg, &st) == -1) {
				fprintf(stderr, "%s is not a valid target directory\n",
					optarg);
				errflg++;
				break;
			}
			target = 1;
			target_dir = optarg;
			/* Remove trailing '/' */
			size_t len = strlen(target_dir);
			if (target_dir[len-1] == '/')
				target_dir[len-1] = '\0';
			break;
		case 'd':
			mod_date = strtol(optarg, NULL, 10);
			break;
		case 'o':
			openpdf = 1;
			break;
		case ':': /* -d or -t without operand */
			fprintf(stderr, "Option -%c requires an operand\n",
				optopt);
			errflg++;
			break;
		case '?':
			fprintf(stderr, "Unrecognized option: -%c\n", optopt);
			errflg++;
		}
	}
	if (errflg) {
		fprintf(stderr, "usage: %s [OPTION]\n", argv[0]);
		fprintf(stderr, "  -d        get the URL of a different issue relative to today's,\n");
		fprintf(stderr, "            e.g. -d -1 for yesterday or -d 1 for tomorrow\n");
		fprintf(stderr, "  -h        print this usage and exit\n");
		fprintf(stderr, "  -o        open the file after download\n");
		fprintf(stderr, "  -t        target directory (instead of the current directory)\n");
		fprintf(stderr, "  -u        print the URL; don't download with libcurl\n");
		fprintf(stderr, "  -v        print version information and exit\n");
		exit(1);
	}
	/* Config */

	/* Fallback values initialized */
	char known_date[11] = "2019-07-07";
	char known_issue[4] = "181";

	char config_path[PATH_SIZE] = "";

	/* If the platform is Windows, place config file in the same directory
	   as the executable */
	if ((0 == strcmp(get_platform_name(),"windows")) ||
	    (0 == strcmp(get_platform_name(),"cygwin"))) {
		strcpy(config_path, "config.txt");
	} else {
		/* Create config directory */
		char *config_dir = "/.config/etc-dl/";
		strcat(config_path, getenv("HOME"));
		strcat(config_path, config_dir);

		if (stat(config_path, &st) == -1) {
			mkdir(config_path, 0700);
			printf("Configuration directory created at \"%s\".\n",
			       config_path);
		}
		strcat(config_path, "config");
	}

	FILE *conf;
	if ((conf = fopen(config_path, "r")) != NULL) {
		int i = 0, c;

		while ((c = fgetc(conf)) != '\n' && i < 10) {
			known_date[i] = c;
			i++;
		}
		known_date[i] = '\0';
		i = 0;
		while ((c = fgetc(conf)) != '\n' && i < 3) {
			known_issue[i] = c;
			i++;
		}
		known_issue[i] = '\0';

		fclose(conf);
	} else {
		printf("Config file not found. Creating it now.\n");
		if ((conf = fopen(config_path, "w")) != NULL) {
			fprintf(conf, "%s", known_date);
			fprintf(conf, "\n");
			fprintf(conf, "%s", known_issue);
			fprintf(conf, "\n");
			fclose(conf);
		}
	}

	/* Verify date and issue format; exit if any of them is invalid */
	for (int i = 0; i < 10; i++) {
		if (isdigit(known_date[i]) == 0 && known_date[i] != '-') {
			fprintf(stderr, "Invalid date format. Please edit \"%s\" and try again.\n",
				config_path);
			exit(1);
		}
	}
	for (int i = 0; i < 3; i++) {
		if (isdigit(known_issue[i]) == 0) {
			fprintf(stderr, "Invalid issue format. Please edit"
				" \"%s\" and try again.\n", config_path);
			exit(1);
		}
	}

	/* printf("Last known date and issue: %s, %s.\n", known_date, known_issue); */

	long known_issue_long = strtol(known_issue, NULL, 10);
	/* generate URL */
	char *etc_url = get_url(mod_date, known_date, known_issue_long);

	/* -p option: print the URL to stdout */
	if (printurl) {
		printf("%s\n", etc_url);

		/* -o option: open the remote URL */
		if (openpdf) {
			open_file(etc_url);
		}
		free(etc_url);
		return 0;
	}
	/* download */

	/* generate path */
	char *path = get_path(target, target_dir, etc_url);

	if (download(path, etc_url) == 0) {
		/* -o option: open the file locally */
		free(etc_url);
		if (openpdf) {
			open_file(path);
		}
		free(path);

		return 0;
	} else {
		/* Retry download using a different URL */
		/* Decrease the month by one and try again */
		printf("Decreasing the month.\n");
		if (decrease_month(etc_url) == -1) {
			fprintf(stderr, "Unable to to decrease the month.\n");
		} else if (download(path, etc_url) == 0) {
			free(etc_url);
			/* -o option: open the file locally */
			if (openpdf) {
				open_file(path);
			}
			free(path);

			return 0;
		}
	}

	printf("Decreasing the issue number.\n");
	free(etc_url);
	free(path);
	for (int i = 0; i < 15; i++) {
		/* Decrease the issue by one and try again */
		known_issue_long--;
		/* regenerate URL */
		char *etc_url = get_url(mod_date, known_date, known_issue_long);
		char *path = get_path(target, target_dir, etc_url);

		if (download(path, etc_url) == 0) {
			if (mod_date >= 0) {
				/* Update config file to
				   last known date and issue*/
				update_config(conf, config_path, etc_url);
			}

			free(etc_url);
			if (openpdf) {
				open_file(path);
			}
			free(path);

			exit(0);
		} else {
			free(etc_url);
			free(path);
		}
	}
	fprintf(stderr, "Unable to guess the URL. Please verify your config file: \"%s\".\n",
		config_path);
	exit(1);
}

static char *get_path(const int target, const char *target_dir,
		      const char *etc_url)
{
	char *path = malloc(sizeof (char) * PATH_SIZE);

	if (target) {
		/* path is target directory plus file name */
		snprintf(path, PATH_SIZE, "%s/%s", target_dir, etc_url+50);
	} else {
		/* path is file name */
		snprintf(path, 36, "%s", etc_url+50);
	}
	return path;
}

static char *get_url(const long mod_date, const char *known_date,
		     const long known_issue_long)
{
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	if (mod_date != 0) {
		/* Modify timeinfo */
		date_plus_days(timeinfo, mod_date);
	}

	/* %Y -- year */
	char year[5];
	strftime(year, 5, "%Y", timeinfo);

	char year_2_digit[3];
	strftime(year_2_digit, 3, "%y", timeinfo);

	/* %d -- month */
	char month[3];
	strftime(month, 3 ,"%m", timeinfo); /* %y%m%d */

	char *etc_url = malloc(sizeof (char) * 81);
	/* e.g. "https://www.etc.se/sites/all/files/papers/2018/07/
	   dagensetc180703nr177.pdf" */

	const char *a = "https://www.etc.se/sites/all/files/papers/";
	const char *b = "dagensetc";

	char ymd[7];
	strftime(ymd,7,"%y%m%d",timeinfo); /* %y%m%d */

	const char *c = ymd; /* %y%m%d */
	const char *d = "nr";

	const long past_date = parse_date(known_date);
	const long selected_date = (long int) time(0);

	/* known_date (e.g. 2018-07-16) was issue number known_issue (e.g. 190).
	   Subtract the difference between the dates from that number. */

	char issue[10];
	snprintf(issue, 10, "%ld", ((mod_date + known_issue_long)
				    - (past_date - selected_date)/(60*60*24)));

	const char *e = ".pdf";

	/* Generate the URL */
	snprintf(etc_url, 81, "%s%s/%s/%s%s%s%s%s",
		 a, year, month, b, c, d, issue, e);

	return etc_url;
}

static void open_file(char *path)
{
	int ret;
	char *cmd[] = { " ", path, (char *) NULL };
	pid_t child;

	/* If the platform is osx, open the file with "open" */
	if (strcmp(get_platform_name(), "osx") == 0) {
		child = fork();
		if (0 == child) {
			ret = execvp("open", cmd);
			if (ret == -1) {
				fprintf(stderr, "Failed to open %s.\n", path);
				free(path);
				exit(1);
			}
		}
	}
	/* For platforms below, open the file with "xdg-open" */
	else if (strcmp(get_platform_name(), "linux") == 0
		 || (strcmp(get_platform_name(), "bsd") == 0)
		 || (strcmp(get_platform_name(), "solaris") == 0)) {
		child = fork();
		if (0 == child) {
			ret = execvp("xdg-open", cmd);
			if (ret == -1) {
				fprintf(stderr, "Failed to open %s.\n", path);
				free(path);
				exit(1);
			}
		}
	/* If the platform is cygwin, open the file with "cygstart" */
	} else if (strcmp(get_platform_name(), "cygwin") == 0) {
		child = fork();
		if (0 == child) {
			ret = execvp("cygstart", cmd);
			if (ret == -1) {
				fprintf(stderr, "Failed to open %s.\n", path);
				free(path);
				exit(1);
			}
		}
	} else {
		fprintf(stderr, "Unable to open the file in %s.",
			get_platform_name());
		fprintf(stderr, "Please report this error to the program's maintainer.");
		exit(1);
	}
	if (child != 0) {
		sleep(1);
		/* Terminate the child process */
		kill(child,SIGTERM);
	}
}

static int download(const char *path, const char *etc_url)
{
	printf("Downloading URL: %s\n", etc_url);

	FILE *file;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);

	/* init the curl session */
	curl_handle = curl_easy_init();

	/* set URL to get here */
	curl_easy_setopt(curl_handle, CURLOPT_URL, etc_url);

	/* Switch on full protocol/debug output while testing */
	curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);

	/* disable progress meter, set to 0L to enable and disable
	   debug output */
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);

	/* we want the headers be sent to the header_callback function */
	curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, header_callback);

	/* send all data to this function  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

	curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1);

	/* check if path already exists. If so, cancel download */
	if (access(path, F_OK) == 0) {
		printf("%s already exists.\n", path);
	} else {
		/* open the file for writing */
		file = fopen(path, "wb");

		if (file) {
			/* write the page body to this file handle */
			curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, file);

			/* get it! */
			res = curl_easy_perform(curl_handle);
			fclose(file);

			/* error */
			if (res != CURLE_OK) {
				fprintf(stderr, "Failed to download %s.\n", path);
				remove(path);

				/* cleanup curl stuff */
				curl_easy_cleanup(curl_handle);
				curl_global_cleanup();

				return 1;
			}

		} else {
			fprintf(stderr, "Unable to open the file %s for writing.\n", path);
		}
	}

	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);
	curl_global_cleanup();

	return 0;
}

static int decrease_month(char *etc_url)
{
	int mon_snd_digit = 0;
	if (sscanf(etc_url+48, "%d", &mon_snd_digit) != 1) {
		return -1;
	}
	if ((strcmp(etc_url+47, "0") == 0) &&
	    (strcmp(etc_url+48, "1") == 0)) {
		return -1;
	} else if ((strcmp(etc_url+47, "1") == 0) &&
		   (strcmp(etc_url+48, "0") == 0)) {
		etc_url[47] = '0'; etc_url[48] = '9';
	} else {
		mon_snd_digit--;
	}
	etc_url[48] = mon_snd_digit + '0';

	return 0;
}

static void update_config(FILE *conf, char *config_path, char *etc_url)
{
	if ((conf = fopen(config_path, "w")) == NULL) {
		fprintf(stderr, "Unable to update the config file \"%s\".\n", config_path);
	} else {
		fprintf(conf, "20%c%c-%c%c-%c%c", etc_url[59], etc_url[60],
			etc_url[61], etc_url[62], etc_url[63], etc_url[64]);
		fprintf(conf, "\n");
		fprintf(conf, "%c", etc_url[67]);
		if (isdigit(etc_url[68]) != 0)
			fprintf(conf, "%c", etc_url[68]);
		if (isdigit(etc_url[69]) != 0)
			fprintf(conf, "%c", etc_url[69]);
		fprintf(conf, "\n");
		fclose(conf);
		printf("The config file has been updated.\n");
	}
}

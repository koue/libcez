/* configfile.c: parsing of config files
 *
 * Copyright (C) 2017-2018 Nikola Kolev <koue@chaosophia.net>
 * Copyright (C) 2008 Lars Hjemli
 *
 * Licensed under GNU General Public License v2
 *   (see COPYING for full license text)
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "cez-config.h"

static int next_char(FILE *f)
{
	int c = fgetc(f);
	if (c=='\r') {
		c = fgetc(f);
		if (c!='\n') {
			ungetc(c, f);
			c = '\r';
		}
	}
	return (c);
}

static void skip_line(FILE *f)
{
	int c;

	while((c=next_char(f)) && c!='\n' && c!=EOF)
		;
}

static int configfile_isspace(char c)
{
	return c==' ' || (c<='\r' && c>='\t');
}

static int read_config_line(FILE *f, char *line, const char **value, int bufsize)
{
	int i = 0, isname = 0;

	*value = NULL;
	while(i<bufsize-1) {
		int c = next_char(f);
		if (!isname && (c=='#' || c==';')) {
			skip_line(f);
			continue;
		}
		if (!isname && configfile_isspace(c))
			continue;

		if (c=='=' && !*value) {
			line[i] = 0;
			*value = &line[i+1];
		} else if (c=='\n' && !isname) {
			i = 0;
			continue;
		} else if (c=='\n' || c==EOF) {
			line[i] = 0;
			break;
		} else {
			line[i]=c;
		}
		isname = 1;
		i++;
	}
	line[i+1] = 0;
	return (i);
}

int configfile_parse(const char *filename, configfile_value_fn fn)
{
	int len;
	char line[256];
	const char *value;
	FILE *f;

	if (!(f = fopen(filename, "r"))) {
		fprintf(stderr, "[ERROR] %s: %s, %s\n", __func__, filename,
								strerror(errno));
		return (-1);
	}
	while((len = read_config_line(f, line, &value, sizeof(line))) > 0) {
		if (value == NULL) {
			fprintf(stderr, "[ERROR] %s: configuration error: %s\n",
								__func__, line);
			return (-1);
		}
		fn(line, value);
	}
	fclose(f);
	return (0);
}

/*
** Copyright (c) 2017-2018 Nikola Kolev <koue@chaosophia.net>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the Simplified BSD License (also
** known as the "2-Clause License" or "FreeBSD License".)

** This program is distributed in the hope that it will be useful,
** but without any warranty; without even the implied warranty of
** merchantability or fitness for a particular purpose.
**
*******************************************************************************
*/

#ifndef _CEZ_CONFIG_H
#define _CEZ_CONFIG_H

/*
** CONFIGFILE
*/
typedef void (*configfile_value_fn)(const char *name, const char *value);
extern int configfile_parse(const char *filename, configfile_value_fn fn);

/*
** CONFIG_ARRAY
*/
void config_array_cb(const char *name, const char *value);
void config_array_print(void);
char *config_array_value_get(const char *name);
void config_array_purge(void);

/*
** CONFIG_QUEUE
*/
void config_queue_cb(const char *name, const char *value);
void config_queue_print(void);
void config_queue_purge(void);
char *config_queue_value_get(const char *name);

#endif

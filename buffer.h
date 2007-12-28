/*
    dbsh - text-based ODBC client
    Copyright (C) 2007 Ben Spencer

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BUFFER_H
#define BUFFER_H

#include <sys/types.h>

struct buffer {
	char *buf;
	size_t len;
	int next;
};

buffer *buffer_alloc(size_t);
void buffer_append(buffer *, char);
char *buffer_dup2str(buffer *);
void buffer_free(buffer *);

#endif

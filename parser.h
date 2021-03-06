/*
    dbsh - text-based ODBC client
    Copyright (C) 2007, 2008 Ben Spencer

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

#ifndef PARSER_H
#define PARSER_H

#include "buffer.h"

typedef enum {
	BUFFER_EMPTY,
	BUFFER_SQL,
	BUFFER_COMMAND
} buffer_type;

struct parsed_line {
	int nchunks;
	char **chunks;
	char *pipeline;
};

buffer_type get_buffer_type(buffer *);
parsed_line *parse_buffer(buffer *);
parsed_line *parse_string(const char *);
void free_parsed_line(parsed_line *);

#endif


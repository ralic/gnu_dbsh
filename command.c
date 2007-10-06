#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "db.h"

db_results *run_command(SQLHDBC conn, char *line)
{
	int i;
	char command[32] = "";
	char *saveptr;
	char *params[4];
	db_results *res = 0;

	for(i = 0; i < 31 && line[i+1] && line[i+1] != ' '; i++) {
		command[i] = tolower(line[i+1]);
	}
	command[i] = 0;

	// TODO: parse properly, allow quoting / escaping etc
	line += i + 1;
	for(i = 0; i < 4; i++) {
		params[i] = strtok_r(line, " ", &saveptr);
		line = 0;
	}

	if(!strcmp(command, "columns")) {
		res = get_columns(conn, params[0], params[1], params[2]);
	} else if(!strcmp(command, "tables")) {
		res = get_tables(conn, params[0], params[1], params[2]);
	} else {
		printf(_("Unrecognised command: %s\n"), command);

	}

	return res;
}

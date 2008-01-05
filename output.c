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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "common.h"
#include "err.h"
#include "output.h"
#include "results.h"


typedef struct {
	int lines;
	int *widths;
	int max_width;
} dim;


#define NULL_DISPLAY "*NULL*"


static wchar_t *strdup2wcs(const char *s)
{
	mbstate_t ps;
	size_t len;
	wchar_t *wcs;

	memset(&ps, 0, sizeof(ps));

	len = mbsrtowcs(0, &s, 0, &ps);
	if(len == -1) err_system();

	if(!(wcs = calloc(len + 1, sizeof(wchar_t)))) err_system();
	mbsrtowcs(wcs, &s, len, &ps);

	return wcs;
}

static dim *get_dimensions(const wchar_t *s)
{
	dim *d;
	const wchar_t *p;
	int line, w;

	if(!(d = calloc(1, sizeof(dim)))) err_system();

	d->lines = 1;
	for(p = s; *p; p++)  if(*p == L'\n') d->lines++;

	if(!(d->widths = calloc(d->lines, sizeof(int)))) err_system();

	line = 0;
	for(p = s; *p; p++) {
		if(*p == L'\n') {
			if(d->widths[line] > d->max_width)
				d->max_width = d->widths[line];
			line++;
		} else if(*p == L'\t') {
			d->widths[line] += 8;
		} else {
			w = wcwidth(*p);
			if(w > 0) d->widths[line] += w;
		}
	}

	if(d->widths[line] > d->max_width) d->max_width = d->widths[line];

	return d;
}

static void free_dimensions(dim *d)
{
	free(d->widths);
	free(d);
}

void output_warnings(results *res, FILE *s)
{
	SQLINTEGER i;
	for(i = 0; i < res->nwarnings; i++) {
		fprintf(s, "%s\n", res->warnings[i]);
	}
}

void output_size_and_time(results *res, FILE *s)
{
	if(res->time_taken.tv_sec || res->time_taken.tv_usec)
		fprintf(s, _("%ld rows in set (%ld.%06lds)\n\n"), res->nrows,
			res->time_taken.tv_sec, res->time_taken.tv_usec);

}

void output_horiz_separator(FILE *s, int col_widths[], SQLSMALLINT ncols)
{
	SQLSMALLINT i;
	int j;

	for(i = 0; i < ncols; i++) {
		fputc('+', s);
		for(j = 0; j < col_widths[i] + 2; j++) fputc('-', s);
	}
	fputs("+\n", s);
}

void output_horiz_row(FILE *s, const char **data,
		      dim **dims, int widths[], SQLSMALLINT ncols)
{
	const char **pos, *p;
	SQLSMALLINT i;
	int j, k, more_lines;

	pos = calloc(ncols, sizeof(char *));
	memcpy(pos, data, ncols * sizeof(char *));

	for(i = 0; i < ncols; i++) if(!pos[i]) pos[i] = NULL_DISPLAY;

	j = 0;
	do {
		more_lines = 0;

		for(i = 0; i < ncols; i++) {

			fputs("| ", s);

			for(p = pos[i]; *p; p++) {
				if(*p == '\n') {
					pos[i] = p + 1;
					more_lines = 1;
					break;
				} else if(*p == '\t') {
					fputs("        ", s);
				} else {
					fputc(*p, s);
				}
			}

			for(k = dims[i]->widths[j]; k <= widths[i]; k++) fputc(' ', s);
		}

		fputs("|\n", s);
		j++;

	} while(more_lines);

	free(pos);
}

void output_horiz(results *res, FILE *s)
{
	SQLSMALLINT i;
	SQLINTEGER j;
	dim **head_dims, ***row_dims;
	int *col_widths;
	wchar_t *wcs;


	if(!(col_widths = calloc(res->ncols, sizeof(int)))) err_system();
	if(!(head_dims = calloc(res->ncols, sizeof(dim *)))) err_system();

	for(i = 0; i < res->ncols; i++) {
		wcs = strdup2wcs(res->cols[i]);
		head_dims[i] = get_dimensions(wcs);
		col_widths[i] = head_dims[i]->max_width;
		free(wcs);
	}

	if(!(row_dims = calloc(res->nrows, sizeof(dim **)))) err_system();

	for(j = 0; j < res->nrows; j++) {
		if(!(row_dims[j] = calloc(res->ncols, sizeof(dim *)))) err_system();
		for(i = 0; i < res->ncols; i++) {
			wcs = strdup2wcs(res->data[j][i] ?
					 res->data[j][i] : NULL_DISPLAY);
			row_dims[j][i] = get_dimensions(wcs);
			if(row_dims[j][i]->max_width > col_widths[i])
				col_widths[i] = row_dims[j][i]->max_width;
			free(wcs);
		}
	}

	output_horiz_separator(s, col_widths, res->ncols);
	output_horiz_row(s, (const char **) res->cols, head_dims, col_widths, res->ncols);
	output_horiz_separator(s, col_widths, res->ncols);
	for(j = 0; j < res->nrows; j++)
		output_horiz_row(s, (const char **) res->data[j], row_dims[j], col_widths, res->ncols);
	output_horiz_separator(s, col_widths, res->ncols);

	free(col_widths);

	for(i = 0; i < res->ncols; i++) free_dimensions(head_dims[i]);
	free(head_dims);

	for(j = 0; j < res->nrows; j++) {
		for(i = 0; i < res->ncols; i++) {
			free_dimensions(row_dims[j][i]);
		}
		free(row_dims[j]);
	}
	free(row_dims);

	output_warnings(res, s);
	output_size_and_time(res, s);
}

void output_vert(results *res, FILE *s)
{
	int col_width;
	SQLSMALLINT i;
	SQLINTEGER j;
	int k;
	char *p;
	wchar_t *wcs;
	dim **head_dims;

	if(!(head_dims = calloc(res->ncols, sizeof(dim *)))) err_system();

	col_width = 0;
	for(i = 0; i < res->ncols; i++) {
		wcs = strdup2wcs(res->cols[i]);
		head_dims[i] = get_dimensions(wcs);
		if(head_dims[i]->max_width > col_width) col_width = head_dims[i]->max_width;
		free(wcs);
	}

	for(j = 0; j < res->nrows; j++) {

		fprintf(s, "******************************** Row %ld ********************************\n", j + 1);

		for(i = 0; i < res->ncols; i++) {

			for(k = 0; k <= col_width - head_dims[i]->widths[0]; k++) fputc(' ', s);

			for(p = res->cols[i]; *p; p++) {
				if(*p == '\t') fputs("        ", s);
				else fputc(*p, s);
			}

			fputs(" | ", s);

			if(res->data[j][i]) {
				for(p = res->data[j][i]; *p; p++) {
					fputc(*p, s);

					if(*p == '\n') {
						for(k = 0; k < col_width + 4; k++) fputc(' ', s);
					}
				}
			} else {
				fputs(NULL_DISPLAY, s);
			}

			fputc('\n', s);
		}
	}

	for(i = 0; i < res->ncols; i++) free_dimensions(head_dims[i]);
	free(head_dims);

	output_warnings(res, s);
	output_size_and_time(res, s);
}

void output_csv_row(FILE *s, const char **data, SQLSMALLINT ncols, char separator, char delimiter)
{
	SQLSMALLINT i;
	const char *p;

	for(i = 0; i < ncols; i++) {

		if(delimiter) fputc(delimiter, s);

		if(data[i]) {
			for(p = data[i]; *p; p++) {
				if(*p == delimiter) fputc(delimiter, s);
				fputc(*p, s);
			}
		}

		if(delimiter) fputc(delimiter, s);

		if(i < ncols - 1) fputc(separator, s);
	}

	fputc('\n', s);
}

void output_csv(results *res, FILE *s, char separator, char delimiter)
{
	SQLINTEGER j;

	output_csv_row(s, (const char **) res->cols, res->ncols, separator, delimiter);
	for(j = 0; j < res->nrows; j++)
		output_csv_row(s, (const char **) res->data[j], res->ncols, separator, delimiter);
}

void output_results(results *res, char mode, FILE *s)
{
	if(mode == 1) mode = *getenv("DBSH_DEFAULT_ACTION");

	do {
		if(res->nrows == -1) {
			output_warnings(res, s);
			fputs(_("Success\n"), s);
		} else if(!res->ncols) {
			output_warnings(res, s);
			fprintf(s, _("%ld rows affected (%ld.%06lds)\n"), res->nrows,
				res->time_taken.tv_sec, res->time_taken.tv_usec);
		} else {
			switch(mode) {
			case 'C':  // CSV
				output_csv(res, s, ',', '"');
				break;
			case 'G':  // Vertical
				output_vert(res, s);
				break;
			case 'H':  // HTML
				fprintf(s, "TODO\n");
				break;
			case 'J':  // JSON
				fprintf(s, "TODO\n");
				break;
			case 'T':  // TSV
				output_csv(res, s, '\t', 0);
				break;
			case 'X':  // XMLS
				fprintf(s, "TODO\n");
				break;
			default:
				output_horiz(res, s);
			}
		}
	} while((res = res->next));
}

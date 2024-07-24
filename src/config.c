/*
 * Filename: config.c
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-07-17
 * Last Modified: 2024-07-24
 *
 * License: See LICENSE file in the project root for license information.
 */

#include "config.h"
#include "errno.h"
#include "toml.h"
#include <stdlib.h>
#include <string.h>

struct tm *parse_str_date(const char *date_str)
{
    // format: "2023-07-21T14:38:00Z"
    struct tm *tm = calloc(sizeof(struct tm), 1); // must be calloc!

    sscanf(date_str, "%d-%d-%dT%d:%d:%d",
           &tm->tm_year, &tm->tm_mon, &tm->tm_mday,
           &tm->tm_hour, &tm->tm_min, &tm->tm_sec);
    tm->tm_year -= 1900; // Year since 1900
    tm->tm_mon -= 1;     // Month from 0 to 11

    return tm;
}

void config_init(config_t *c, char *path)
{
    FILE *fp;
    char errbuf[200];

    fp = fopen(path, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "cannot open sample.toml - %s", strerror(errno));
    }

    c->table = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!c->table)
    {
        fprintf(stderr, "cannot parse - %s", errbuf);
    }

    toml_table_t *files = toml_table_in(c->table, "files");
    toml_table_t *input = toml_table_in(files, "input");
    toml_array_t *data = toml_array_in(input, "data");
    toml_datum_t sections = toml_string_in(input, "sections");
    toml_datum_t variants = toml_string_in(input, "variants");
    toml_datum_t points = toml_string_in(input, "points");

    toml_table_t *output = toml_table_in(files, "output");
    toml_datum_t results_s = toml_string_in(output, "sections");
    toml_datum_t results_p = toml_string_in(output, "points");

    // toml_table_t *runtime = toml_table_in(c->table, "runtime");
    // toml_datum_t threads = toml_int_in(runtime, "threads");
    // toml_datum_t memory = toml_int_in(runtime, "memory");

    toml_table_t *analysis = toml_table_in(c->table, "analysis");

    toml_datum_t interval = toml_int_in(analysis, "interval");
    c->interval = interval.u.i;

    const char *s;
    // parse str to date
    s = toml_raw_in(analysis, "from");
    struct tm *from_date = parse_str_date(s);
    s = toml_raw_in(analysis, "to");
    struct tm *to_date = parse_str_date(s);

    // normalize and get timestamp from dates
    c->from_t = mktime(from_date);
    c->to_t = mktime(to_date);

    c->from_date = from_date;
    c->to_date = to_date;

    c->input_sections = sections.u.s;
    c->input_variants = variants.u.s;
    c->input_points = points.u.s;
    c->output_sections = results_s.u.s;
    c->output_points = results_p.u.s;

    // c->threads = threads.u.i;
    // c->memory = threads.u.i;

    c->total_data_files = toml_array_nelem(data);
    c->data_files = (char **)malloc(sizeof(char *) * c->total_data_files);

    for (int i = 0; i < c->total_data_files; i++)
    {
        toml_datum_t elem = toml_string_at(data, i);
        c->data_files[i] = elem.u.s;
    }
}

void config_destroy(config_t *c)
{
    toml_free(c->table);
    free(c->input_sections);
    free(c->input_variants);
    free(c->input_points);
    free(c->output_sections);
    free(c->output_points);
    for (int i = 0; i < c->total_data_files; i++)
    {
        free(c->data_files[i]);
    }
    free(c->data_files);
}

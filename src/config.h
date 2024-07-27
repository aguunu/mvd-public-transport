/*
 * Filename: config.h
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-07-17
 * Last Modified: 2024-07-27
 *
 * License: See LICENSE file in the project root for license information.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "time.h"
#include "toml.h"

typedef struct
{
    toml_table_t *table;
    char **data_files;
    int total_data_files;
    char *input_sections;
    char *input_variants;
    char *input_points;
    char *output_sections;
    char *output_points;
    struct tm *from_date;
    struct tm *to_date;
    time_t from_t;
    time_t to_t;
    int interval;
    int save_results;
    // int threads;
    // int memory;
} config_t;

void config_init(config_t *c, char *path);

void config_destroy(config_t *c);

#endif

/*
 * Filename: config.h
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-07-17
 * Last Modified: 2024-07-17
 *
 * License: See LICENSE file in the project root for license information.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "toml.h"

typedef struct
{
    toml_table_t *table;
    char **data_files;
    int total_data_files;
    char *input_sections;
    char *input_variants;
    char *input_points;
    char *output_results;
    // int threads;
    // int memory;
} config_t;

void config_init(config_t *c, char *path);

void config_destroy(config_t *c);

#endif

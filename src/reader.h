/*
 * Filename: reader.h
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-06-12
 * Last Modified: 2024-07-18
 *
 * License: See LICENSE file in the project root for license information.
 */

#ifndef READER_H
#define READER_H

#include "bus-record.h"
#include "config.h"
#include <stdio.h>

typedef struct
{
    FILE **files;
    int cursor;
    int n_files;
} reader_t;

int reader_init(reader_t *reader, config_t *config);

void reader_destroy(reader_t *reader);

int reader_read(reader_t *reader, record_t *data);

#endif

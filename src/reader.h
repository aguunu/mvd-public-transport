/*
 * Filename: reader.h
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-06-12
 * Last Modified: 2024-06-12
 *
 * License: See LICENSE file in the project root for license information.
 */

#ifndef READER_H
#define READER_H

#include "bus-record.h"
#include <stdio.h>

typedef struct
{
    FILE **files;
    int cursor;
    int n_files;
} DataReader;

int reader_init(DataReader *reader, char *file_path);

void reader_destroy(DataReader *reader);

int reader_read(DataReader *reader, BusRecord *data);

#endif

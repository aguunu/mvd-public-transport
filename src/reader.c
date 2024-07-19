/*
 * Filename: reader.c
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

#include "reader.h"
#include "colors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void reader_destroy(reader_t *reader)
{
    for (int i = reader->cursor; i < reader->n_files; i++)
    {
        fclose(reader->files[i]);
    }
    free(reader->files);
}

int reader_init(reader_t *reader, config_t *config)
{
    // Init structure
    reader->files = NULL;
    reader->cursor = 0;
    reader->n_files = config->total_data_files;

    // Create array of pointer to files
    reader->files = (FILE **)malloc(sizeof(FILE *) * reader->n_files);
    if (reader->files == NULL)
    {
        fprintf(stderr, "Failed to alloc\n");
        return 1;
    }

    // Open files
    for (int i = 0; i < reader->n_files; i++)
    {
        char *path = config->data_files[i];
        reader->files[i] = fopen(config->data_files[i], "r");
        if (reader->files[i] == NULL)
        {
            fprintf(stderr, "Failed to open file %s\n", path);
            return 1;
        }
        fprintf(stdout, GREEN "+ Data File: \"%s\"\n" NO_COLOR, path);
    }

    return 0;
}

int reader_read(reader_t *reader, record_t *data)
{
    // Check if no files left
    if (reader->cursor == reader->n_files)
    {
        return 1;
    }

    // Read data from current file
    int _tmp;
    FILE *ptr_file = reader->files[reader->cursor];
    int got = fscanf(ptr_file, "%ld,%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%f",
                     &data->timestamp,
                     &data->company,
                     &data->hh,
                     &data->mm,
                     &data->id_bus,
                     &data->variant,
                     &data->type,
                     &_tmp,
                     &_tmp,
                     &_tmp,
                     &data->p.lon,
                     &data->p.lat);

    // Change file.
    if (got == 0 || got == -1)
    {
        // Close current file
        fclose(reader->files[reader->cursor]);
        // Point to next file
        reader->cursor++;
    }
    else if (got != 12)
    {
        fprintf(stderr, "Expected %d, got %d values from\n", 12, got);
        return 1;
    }

    return 0;
}

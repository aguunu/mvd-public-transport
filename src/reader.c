/*
 * Filename: reader.c
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-06-12
 * Last Modified: 2024-06-27
 *
 * License: See LICENSE file in the project root for license information.
 */

#include "reader.h"
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

int reader_init(reader_t *reader, char *file_path)
{
    // Init structure
    reader->files = NULL;
    reader->cursor = 0;
    reader->n_files = 0;

    // Create array of pointer to files
    int arr_size = 2;
    reader->files = (FILE **)malloc(sizeof(FILE *) * arr_size);
    if (reader->files == NULL)
    {
        fprintf(stderr, "Failed to alloc\n");
        return 1;
    }

    // Open main file
    FILE *ptr_file = fopen(file_path, "r");
    if (ptr_file == NULL)
    {
        fprintf(stderr, "Failed to open file %s\n", file_path);
        return 1;
    }

    // Open files
    char line[1024];
    while (fgets(line, sizeof(line), ptr_file))
    {
        size_t length = strlen(line);

        if (length > 0 && line[length - 1] == '\n')
        {
            line[length - 1] = '\0';
        }

        printf("%s\n", line);
        FILE *ptr_new_file = fopen(line, "r");
        if (ptr_new_file == NULL)
        {
            fprintf(stderr, "Failed to open file %s\n", line);
            return 1;
        }
        if (arr_size == reader->n_files)
        {
            arr_size *= 2;
            reader->files = (FILE **)realloc(reader->files, sizeof(FILE *) * arr_size);
            if (reader->files == NULL)
            {
                fprintf(stderr, "Failed to realloc\n");
                fclose(ptr_file);
                return 1;
            }
        }
        reader->files[reader->n_files] = ptr_new_file;
        reader->n_files++;
    }

    // Close file
    fclose(ptr_file);

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

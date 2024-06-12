/*
 * Filename: main.c
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

#include "args.h"
#include "bus-record.h"
#include "colors.h"
#include "latlon.h"
#include "reader.h"
#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_DATA_PER_TRIP 4096
#define MAX_VARIANT 10000
#define MAX_HH 24
#define MAX_MM 60
#define TRIP_LENGTH_THRESHOLD 60 * 60 * 3

typedef struct
{
    int n;
    BusRecord records[MAX_DATA_PER_TRIP];
} Entry;

void do_work(Entry *buf)
{
    // do work
    // ...
    free(buf);
}

int master(DataReader *reader)
{
#pragma omp parallel
    {
#pragma omp single
        {
            int processed = 0;
            int count = 0;
            int workers = 0;

            Entry **map = (Entry **)malloc(sizeof(Entry *) * (MAX_VARIANT * MAX_HH * MAX_MM));
            for (int i = 0; i < (MAX_VARIANT * MAX_HH * MAX_MM); i++)
            {
                map[i] = NULL;
            }

            BusRecord data;
            int reader_err;

            while ((reader_err = reader_read(reader, &data)) == 0)
            {
                count += 1;

                // TODO: Chequear si tenemos los datos correspondientes
                // para data.variant.
                if (data.variant >= MAX_VARIANT)
                {
                    continue;
                }

                // TODO: Filtrar datos erroneos
                if (data.hh >= MAX_HH || data.mm >= MAX_MM)
                {
                    continue;
                }

                assert(data.hh < MAX_HH);
                assert(data.mm < MAX_MM);
                assert(data.variant < MAX_VARIANT);

                int idx = data.variant * (MAX_HH * MAX_MM) + data.hh * MAX_MM + data.mm;

                Entry *buf = NULL;
                if (map[idx] != NULL)
                {
                    BusRecord *prev = &map[idx]->records[0];

                    if (map[idx]->n == MAX_DATA_PER_TRIP || data.timestamp - prev->timestamp > TRIP_LENGTH_THRESHOLD)
                    {
                        buf = map[idx];
                    }
                }

                if (buf != NULL || !map[idx])
                {
                    map[idx] = (Entry *)malloc(sizeof(Entry));
                    map[idx]->n = 0;
                }
                assert(buf != map[idx]);
                assert(map[idx]->n < MAX_DATA_PER_TRIP);

                // Add data
                processed += 1;
                map[idx]->records[map[idx]->n] = data;
                map[idx]->n++;

                if (buf)
                {
#pragma omp task default(none) untied firstprivate(buf)
                    {
                        do_work(buf);
                    }
                    workers += 1;
                }
            }

            for (int i = 0; i < (MAX_VARIANT * MAX_MM * MAX_HH); i++)
            {
                if (map[i] != NULL)
                {
                    Entry *buf = map[i];
#pragma omp task default(none) untied firstprivate(workers) firstprivate(buf)
                    {
                        assert(buf != NULL);
                        do_work(buf);
                    }
                    workers += 1;
                }
            }
#pragma omp taskwait
            free(map);
            printf("Reads: %d\nProcessed: %d\nTasks: %d\n", count, processed, workers);

        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    options_t options;
    options_parser(argc, argv, &options);

#ifdef DEBUG
    fprintf(stdout, BLUE "Command line options:\n" NO_COLOR);
    fprintf(stdout, BROWN "help: %d\n" NO_COLOR, options.help);
    fprintf(stdout, BROWN "version: %d\n" NO_COLOR, options.version);
    fprintf(stdout, BROWN "use colors: %d\n" NO_COLOR, options.use_colors);
    fprintf(stdout, BROWN "filename: %s\n" NO_COLOR, options.file_name);
    fprintf(stdout, BROWN "n-threads: %d\n" NO_COLOR, options.n_threads);
#endif

    omp_set_num_threads(options.n_threads);

    DataReader reader;
    int err = reader_init(&reader, options.file_name);

    master(&reader);

    reader_destroy(&reader);

    return EXIT_SUCCESS;
}

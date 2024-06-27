/*
 * Filename: master.c
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-06-18
 * Last Modified: 2024-06-27
 *
 * License: See LICENSE file in the project root for license information.
 */

#include "master.h"
#include "bus-record.h"
#include "model.h"
#include "reader.h"
#include "slave.h"
#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HH 24
#define MAX_MM 60
#define TRIP_LENGTH_THRESHOLD 60 * 60 * 3

void work_map_init(work_t **map)
{
    int n = MAX_VARIANT * MAX_HH * MAX_MM;
    map = (work_t **)malloc(sizeof(work_t *) * n);
    for (int i = 0; i < n; i++)
    {
        map[i] = NULL;
    }
}

int work_map_add(work_t **map, record_t data, work_t *trip)
{
    assert(data.hh < MAX_HH);
    assert(data.mm < MAX_MM);
    assert(data.variant < MAX_VARIANT);

    int idx = data.variant * (MAX_HH * MAX_MM) + data.hh * MAX_MM + data.mm;

    if (map[idx] != NULL)
    {
        record_t *prev = &map[idx]->records[0];
        if (prev->id_bus != data.id_bus)
        {
            return 1;
        }

        if (map[idx]->n == MAX_DATA_PER_TRIP || data.timestamp - prev->timestamp > TRIP_LENGTH_THRESHOLD)
        {
            trip = map[idx];
        }
    }

    if (trip != NULL || !map[idx])
    {
        map[idx] = (work_t *)malloc(sizeof(work_t));
        map[idx]->n = 0;
    }
    assert(trip != map[idx]);
    assert(map[idx]->n < MAX_DATA_PER_TRIP);

    // Add data
    map[idx]->records[map[idx]->n] = data;
    map[idx]->n++;

    return 0;
}

int master(reader_t *reader, model_t *model)
{
#pragma omp parallel
    {
#pragma omp single
        {
            int processed = 0;
            int count = 0;
            int workers = 0;

            int n = MAX_VARIANT * MAX_HH * MAX_MM;
            work_t **map = (work_t **)malloc(sizeof(work_t *) * n);
            for (int i = 0; i < n; i++)
            {
                map[i] = NULL;
            }

            record_t data;
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

                work_t *trip = NULL;
                if (work_map_add(map, data, trip) != 0)
                {
                    // If error while adding continue
                    continue;
                }
                processed++;

                if (trip)
                {
#pragma omp task default(none) firstprivate(trip) firstprivate(model)
                    {
                        int v_code = trip->records[0].variant;
                        do_work(trip, model->variants[v_code], &model->critical_points);
                    }
                    workers += 1;
                }
            }

            for (int i = 0; i < (MAX_VARIANT * MAX_MM * MAX_HH); i++)
            {
                if (map[i] != NULL)
                {
                    work_t *trip = map[i];
#pragma omp task default(none) firstprivate(trip) firstprivate(model)
                    {
                        assert(trip != NULL);
                        int v_code = trip->records[0].variant;
                        do_work(trip, model->variants[v_code], &model->critical_points);
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

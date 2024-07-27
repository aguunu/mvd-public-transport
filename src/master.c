/*
 * Filename: master.c
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-06-18
 * Last Modified: 2024-07-27
 *
 * License: See LICENSE file in the project root for license information.
 */

#include "master.h"
#include "colors.h"
#include "model.h"
#include "reader.h"
#include "slave.h"
#include "work.h"
#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int _master(reader_t *reader, model_t *model)
{
    int processed = 0;
    int count = 0;
    int tasks = 0;

    work_map_t *map = work_map_init();

    record_t data;
    reader_code reader_code;

    while ((reader_code = reader_read(reader, &data)) != READER_NO_RECORDS_LEFT)
    {
        count += 1;
        if (reader_code != READER_OK)
        {
            continue;
        }

        // check if data is inside analysis limits
        if (data.timestamp < model->config->from_t || data.timestamp >= model->config->to_t)
        {
            continue;
        }

        // TODO: Chequear si tenemos los datos correspondientes
        // para data.variant.
        if (data.variant >= MAX_VARIANT)
        {
            continue;
        }

        work_t *trip = work_map_add(map, &data);
        processed++;

        if (trip != NULL)
        {
#pragma omp task default(none) priority(5) firstprivate(trip) firstprivate(model)
            {
                slave(trip, model);
            }
            tasks += 1;
        }
    }

    for (int i = 0; i < 24 * 60 * MAX_VARIANT; i++)
    {
        work_t *trip = map->arr[i];
        if (trip != NULL)
        {
#pragma omp task default(none) priority(5) firstprivate(trip) firstprivate(model)
            {
                slave(trip, model);
            }
        }
    }

    work_map_destroy(map);

    printf(MAGENTA "Master has finished! Waiting for tasks...\n" NO_COLOR);

#pragma omp taskwait
    printf("Reads: %d\nProcessed: %d\nTasks: %d\n", count, processed, tasks);

    return 0;
}

int master(reader_t *reader, model_t *model)
{
    int err;
#pragma omp parallel
#pragma omp master
    {
#pragma omp task default(none) untied priority(10) shared(err) firstprivate(reader) firstprivate(model)
        err = _master(reader, model);
    }
    return err;
}

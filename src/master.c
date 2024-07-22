/*
 * Filename: master.c
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-06-18
 * Last Modified: 2024-07-22
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

int master(reader_t *reader, model_t *model)
{
#pragma omp parallel
    {
#pragma omp single
        {
            int processed = 0;
            int count = 0;
            int workers = 0;

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
#pragma omp task default(none) firstprivate(trip) firstprivate(model)
                    {
                        slave(trip, model);
                    }
                    workers += 1;
                }
            }

            for (int i = 0; i < WORK_MAP_ENTRIES; i++)
            {
                work_entry_v2_t *entry = &map->entries[i];
                for (int j = 0; j < entry->n; j++)
                {
                    work_t *trip = entry->work[j];
                    if (trip != NULL)
#pragma omp task default(none) firstprivate(trip) firstprivate(model)
                    {
                        slave(trip, model);
                    }
                }
                workers += 1;
            }

            work_map_destroy(map);

            printf(MAGENTA "Master has finished! Waiting for tasks...\n" NO_COLOR);

#pragma omp taskwait
            printf("Reads: %d\nProcessed: %d\nTasks: %d\n", count, processed, workers);
        }
    }

    return 0;
}

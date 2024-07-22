/*
 * Filename: work.h
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-07-20
 * Last Modified: 2024-07-22
 *
 * License: See LICENSE file in the project root for license information.
 */

#include "work.h"
#include "assert.h"
#include "stdlib.h"

work_map_t *work_map_init()
{
    work_map_t *map = (work_map_t *)malloc(sizeof(work_map_t));
    assert(map != NULL);

    for (int i = 0; i < WORK_MAP_ENTRIES; i++)
    {
        work_entry_v2_t *entry = &map->entries[i];

        entry->n = 0;
        entry->max = WORK_ENTRY_INIT_MAX;
        entry->work = (work_t **)malloc(sizeof(work_t *) * entry->max);
        assert(entry->work != NULL);

        for (int j = entry->n; j < entry->max; j++)
        {
            entry->work[j] = NULL;
        }
    }
    return map;
}

void work_map_destroy(work_map_t *map)
{
    for (int i = 0; i < WORK_MAP_ENTRIES; i++)
    {
        free(map->entries[i].work);
    }
    free(map);
}

void work_push(work_t *work, record_t *data)
{
    assert(work->n < MAX_DATA_PER_TRIP);
    int lst_idx = work->n;
    work->records[lst_idx].timestamp = data->timestamp;
    work->records[lst_idx].p = data->p;
    work->n++;
}

int should_be_proccesed(work_t *work, time_t current_t)
{
    assert(work != NULL);
    if (work->n > 0)
    {
        // check if trip to process
        time_t delta_t = current_t - work->records[0].timestamp;
        // assert(delta_t >= 0);
        if (delta_t <= 0 || delta_t > TRIP_LENGTH_THRESHOLD || work->n == MAX_DATA_PER_TRIP)
        {
            return 1;
        }
    }
    return 0;
}

int work_cmp(work_t *data1, record_t *data2)
{
    return (
        data1->id_bus == data2->id_bus &&
        data1->hh == data2->hh &&
        data1->mm == data2->mm &&
        data1->variant == data2->variant &&
        data1->type == data2->type &&
        data1->company == data2->company &&
        data1->subsystem == data2->subsystem &&
        data1->destination == data2->destination);
}

int hash_record(record_t *r)
{
    return r->variant * (r->hh * 3600 + r->mm * 60);
}

work_t *work_map_add(work_map_t *map, record_t *data)
{

    assert(map != NULL);
    assert(map->entries != NULL);

    // compute index
    int idx = hash_record(data) & (WORK_MAP_ENTRIES - 1);
    assert(idx >= 0);
    assert(idx < WORK_MAP_ENTRIES);

    work_entry_v2_t *entry = &map->entries[idx];

    // find entry cursor
    int cursor = -1;
    for (int i = 0; i < entry->n; i++)
    {
        work_t *w = entry->work[i];
        if (w != NULL && work_cmp(w, data))
        {
            cursor = i;
            break;
        }
        else if (w == NULL)
        {
            // null cursor
            cursor = i;
        }
    }

    // if not cursor (not found neither null entry)
    if (cursor == -1)
    {
        cursor = entry->n;
        entry->max += 128;
        // assert(entry->max <= 4096);
        entry->work = realloc(entry->work, sizeof(work_t *) * entry->max);
        assert(entry->work != NULL);
        for (int i = entry->n; i < entry->max; i++)
        {
            entry->work[i] = NULL;
        }
        entry->n++;
    }

    assert(cursor != -1);
    assert(cursor < entry->max);
    if (entry->work[cursor] == NULL)
    {
        entry->work[cursor] = malloc(sizeof(work_t));
        // copy data from record_t to work_t
        entry->work[cursor]->id_bus = data->id_bus;
        entry->work[cursor]->variant = data->variant;
        entry->work[cursor]->company = data->company;
        entry->work[cursor]->hh = data->hh;
        entry->work[cursor]->mm = data->mm;
        entry->work[cursor]->type = data->type;
        entry->work[cursor]->destination = data->destination;
        entry->work[cursor]->subsystem = data->subsystem;
        entry->work[cursor]->n = 0;
    }

    assert(entry->work[cursor] != NULL);

    if (should_be_proccesed(entry->work[cursor], data->timestamp))
    {
        work_t *ready_work = entry->work[cursor];
        entry->work[cursor] = NULL;
        return ready_work;
    }

    // add data to current work
    work_push(entry->work[cursor], data);

    return NULL;
}

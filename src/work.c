/*
 * Filename: work.h
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-07-20
 * Last Modified: 2024-07-27
 *
 * License: See LICENSE file in the project root for license information.
 */

#include "work.h"
#include "assert.h"
#include "model.h"
#include "stdlib.h"

work_map_t *work_map_init()
{
    work_map_t *map = (work_map_t *)malloc(sizeof(work_map_t));
    assert(map != NULL);

    map->arr = calloc(24 * 60 * MAX_VARIANT, sizeof(work_t *));

    return map;
}

void work_map_destroy(work_map_t *map)
{
    free(map->arr);
    free(map);
}

int record_from_work(work_t *work, record_t *r)
{
    int eq = 1;
    eq &= work->id_bus == r->id_bus;
    eq &= work->variant == r->variant;
    eq &= work->company == r->company;
    eq &= work->hh == r->hh;
    eq &= work->mm == r->mm;
    eq &= work->type == r->type;
    eq &= work->destination == r->destination;
    eq &= work->subsystem == r->subsystem;

    return eq;
}

int is_diff_trip(work_t *work, record_t *r)
{
    assert(work != NULL);
    assert(work->n > 0);

    time_t current_t = r->timestamp;
    time_t delta_t = current_t - work->records[0].timestamp;

    if (
        !record_from_work(work, r) ||
        delta_t <= 0 ||
        delta_t > TRIP_LENGTH_THRESHOLD ||
        work->n == MAX_DATA_PER_TRIP)
    {
        return 1;
    }

    return 0;
}

work_t *work_map_add(work_map_t *map, record_t *data)
{
    int frequency = data->hh * 60 + data->mm;
    int mm_per_day = 24 * 60;
    int idx = mm_per_day * data->variant + frequency;

    assert(idx < mm_per_day * MAX_VARIANT);

    if (map->arr[idx] == NULL)
    {
        map->arr[idx] = malloc(sizeof(work_t));
        map->arr[idx]->n = 0;
        map->arr[idx]->id_bus = data->id_bus;
        map->arr[idx]->variant = data->variant;
        map->arr[idx]->company = data->company;
        map->arr[idx]->hh = data->hh;
        map->arr[idx]->mm = data->mm;
        map->arr[idx]->type = data->type;
        map->arr[idx]->destination = data->destination;
        map->arr[idx]->subsystem = data->subsystem;
    }

    assert(map->arr[idx] != NULL);

    // check if work buf from different trip
    if (map->arr[idx]->n != 0 && is_diff_trip(map->arr[idx], data))
    {
        work_t *work = map->arr[idx];
        map->arr[idx] = NULL;

        return work;
    }

    assert(map->arr[idx] != NULL);
    int n = map->arr[idx]->n;
    map->arr[idx]->records[n].timestamp = data->timestamp;
    map->arr[idx]->records[n].p = data->p;
    map->arr[idx]->n++;

    assert(map->arr[idx]->id_bus == data->id_bus);
    assert(map->arr[idx]->variant == data->variant);
    assert(map->arr[idx]->company == data->company);
    assert(map->arr[idx]->hh == data->hh);
    assert(map->arr[idx]->mm == data->mm);
    assert(map->arr[idx]->type == data->type);
    assert(map->arr[idx]->destination == data->destination);
    assert(map->arr[idx]->subsystem == data->subsystem);

    // check if work buf is full
    if (map->arr[idx]->n == MAX_DATA_PER_TRIP)
    {
        work_t *work = map->arr[idx];
        map->arr[idx] = NULL;

        return work;
    }

    return NULL;
}

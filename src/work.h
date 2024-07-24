/*
 * Filename: work.h
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-07-20
 * Last Modified: 2024-07-24
 *
 * License: See LICENSE file in the project root for license information.
 */

#ifndef WORK_H
#define WORK_H

#define MAX_HH 24
#define MAX_MM 60
#define TRIP_LENGTH_THRESHOLD 60 * 60 * 3

#define MAX_DATA_PER_TRIP 1024

#include "bus-record.h"
#include "latlon.h"

typedef struct
{
    time_t timestamp;
    point_t p;
} mini_record_t;

typedef struct
{
    int id_bus;
    int variant;
    int company;
    int hh;
    int mm;
    int type;
    int destination;
    int subsystem;
    int n;
    mini_record_t records[MAX_DATA_PER_TRIP];
} work_t;

typedef struct
{
    work_t **arr;
} work_map_t;

work_map_t *work_map_init();
work_t *work_map_add(work_map_t *map, record_t *data);
void work_map_destroy(work_map_t *map);

#endif

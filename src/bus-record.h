/*
 * Filename: bus-record.h
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-06-12
 * Last Modified: 2024-07-22
 *
 * License: See LICENSE file in the project root for license information.
 */

#ifndef BUS_RECORD_H
#define BUS_RECORD_H

#include "latlon.h"
#include "section.h"
#include <time.h>

typedef enum
{
    DIFERENCIAL = 0,
    LOCAL = 1,
    URBANA = 2,
    METROPOLITANA = 3,
    INTER_URBANA = 4,
    CENTRICA_MVD = 5,
} variant_type;

typedef struct
{
    time_t timestamp;
    int id_bus;
    int variant;
    int company;
    int hh;
    int mm;
    int type;
    int destination;
    int subsystem;
    point_t p;
} record_t;

int is_valid(record_t *r);

#endif

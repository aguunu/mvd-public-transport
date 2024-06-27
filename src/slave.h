/*
 * Filename: slave.h
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

#ifndef SLAVE_H
#define SLAVE_H

#include "bus-record.h"
#include "model.h"

#define MAX_DATA_PER_TRIP 4096

#define RADIUS_THRESHOLD_M 25
#define SPEED_LIMIT_KM_H 45

typedef struct
{
    int n;
    record_t records[MAX_DATA_PER_TRIP];
} work_t;

void do_work(work_t *buf, variant_t *info, crit_points_t *critical_points);

#endif

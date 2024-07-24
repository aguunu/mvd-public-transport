/*
 * Filename: section.h
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-06-12
 * Last Modified: 2024-07-24
 *
 * License: See LICENSE file in the project root for license information.
 */

#ifndef CRITICAL_POINTS_H
#define CRITICAL_POINTS_H

#include "latlon.h"
#include "section.h"
#include <omp.h>

typedef struct
{
    int id;
    point_t p;
    int type;
    metrics_t *metrics;
} crit_point_t;

typedef struct
{
    crit_point_t *p;
    int n;
} crit_points_t;

#endif

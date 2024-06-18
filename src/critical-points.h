
/*
 * Filename: section.h
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

#ifndef CRITICAL_POINTS_H
#define CRITICAL_POINTS_H

#include "latlon.h"
#include "section.h"
#include <omp.h>

typedef struct
{
    int id;
    Point p;
    int type;
    Metrics metrics;
    omp_lock_t lock;
} CriticalPoint;

typedef struct
{
    CriticalPoint *p;
    int n;
} CriticalPoints;

#endif

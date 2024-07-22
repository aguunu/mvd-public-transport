/*
 * Filename: section.h
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

#ifndef SECTION_H
#define SECTION_H

#include "latlon.h"
#include <omp.h>

#define HISTOGRAM_BINS 60

typedef struct
{
    int histogram[HISTOGRAM_BINS];
} metrics_t;

typedef struct
{
    int id;
    int n_path;
    int length;
    point_t *path;
    metrics_t *metrics;
} section_t;

float section_length(section_t *ptr_s);
float section_d2p(section_t *ptr_s, point_t p);

#endif

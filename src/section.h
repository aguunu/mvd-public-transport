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

#ifndef SECTION_H
#define SECTION_H

#include "latlon.h"
#include <omp.h>

#define HISTOGRAM_BINS 90

typedef struct
{
    omp_lock_t lock;
    int histogram[HISTOGRAM_BINS]; // from 0 to 90km/h
} Metrics;

typedef struct
{
    int id;
    int n_path;
    Point *path;
    Metrics metrics;
} Section;

float section_length(Section *ptr_s);
float section_d2p(Section *ptr_s, Point p);
void section_add_data(Section *ptr_s, float speed);

#endif

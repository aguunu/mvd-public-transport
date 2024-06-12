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

typedef struct
{
    int histogram[90]; // from 0 to 90km/h
} Metrics;

typedef struct
{
    int id;
    Point *path;
    int n_path;
    Metrics metrics;
} Section;

float section_length(Section *ptr_s);
float section_d2p(Section *ptr_s, Point p);
void section_add_data(Section *ptr_s, float speed);

#endif

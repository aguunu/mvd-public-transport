/*
 * Filename: model.h
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

#ifndef MODEL_H
#define MODEL_H

#include "critical-points.h"
#include "section.h"

#define MAX_SECTIONS_PER_VARIANT 256
#define MAX_SECTIONS 6000
#define MAX_VARIANT 10000

typedef struct
{
    section_t *sections[MAX_SECTIONS_PER_VARIANT];
    int n_sections;
} variant_t;

typedef struct
{
    variant_t **variants;
    section_t **sections;
    crit_points_t critical_points;
} model_t;

void load_model(model_t *model);
void save_model(model_t *model);
void destroy_model(model_t *model);

#endif

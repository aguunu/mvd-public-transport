/*
 * Filename: model.h
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-06-18
 * Last Modified: 2024-07-24
 *
 * License: See LICENSE file in the project root for license information.
 */

#ifndef MODEL_H
#define MODEL_H

#include "config.h"
#include "critical-points.h"
#include "section.h"

#define MAX_VARIANT 10000

typedef struct
{
    section_t **sections;
    int n_sections;
} variant_t;

typedef struct
{
    section_t *sections;
    int n;
} sections_t;

typedef struct
{
    config_t *config;
    variant_t **variants;
    sections_t sections;
    crit_points_t critical_points;
} model_t;

int load_model(model_t *model, config_t *config);
void save_model(model_t *model);
void destroy_model(model_t *model);

#endif

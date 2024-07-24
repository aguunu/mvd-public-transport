/*
 * Filename: slave.h
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

#ifndef SLAVE_H
#define SLAVE_H

#include "model.h"
#include "work.h"

#define RADIUS_THRESHOLD_M 30
#define SPEED_LIMIT_KM_H 45

void slave(work_t *buf, model_t *model);

#endif

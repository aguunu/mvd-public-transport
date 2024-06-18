/*
 * Filename: master.h
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-06-18
 * Last Modified: 2024-06-18
 *
 * License: See LICENSE file in the project root for license information.
 */

#ifndef MASTER_H
#define MASTER_H

#include "model.h"
#include "reader.h"

int master(DataReader *reader, model_t *model);

#endif

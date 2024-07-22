/*
 * Filename: bus-record.c
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

#include "bus-record.h"

int is_valid(record_t *r)
{
    int valid = 1;

    // check frequency
    valid &= r->mm >= 0 && r->mm < 60;
    valid &= r->hh >= 0 && r->hh < 24;

    return valid;
}

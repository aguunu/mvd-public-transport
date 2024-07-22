/*
 * Filename: section.c
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

#include "section.h"
#include "latlon.h"

float section_length(section_t *ptr_s)
{
    float length = 0;
    for (int i = 0; i < ptr_s->n_path - 1; i++)
    {
        point_t p1 = ptr_s->path[i];
        point_t p2 = ptr_s->path[i + 1];

        length += distance(p1, p2);
    }
    return length;
}

float section_d2p(section_t *ptr_s, point_t p)
{
    float min_d = -1;
    for (int i = 0; i < ptr_s->n_path - 1; i++)
    {
        point_t p1 = ptr_s->path[i];
        point_t p2 = ptr_s->path[i + 1];
        float d = crossarc(p1, p2, p);
        if (d < min_d || min_d == -1)
        {
            min_d = d;
        }
    }
    return min_d;
}

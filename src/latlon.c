/*
 * Filename: latlon.c
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-06-12
 * Last Modified: 2024-06-27
 *
 * License: See LICENSE file in the project root for license information.
 */

#include "latlon.h"
#include <math.h>

#define R 6.371e6 // meters

float deg2rad(float deg)
{
    return deg * (M_PI / 180);
}

float distance(point_t p1, point_t p2)
{
    float phi1 = deg2rad(p1.lat);
    float phi2 = deg2rad(p2.lat);
    float delta_phi = deg2rad(p2.lat - p1.lat);
    float delta_lambda = deg2rad(p2.lon - p1.lon);

    float a = sinf(delta_phi / 2) * sinf(delta_phi / 2) + cosf(phi1) * cosf(phi2) * sinf(delta_lambda / 2) * sinf(delta_lambda / 2);
    float c = 2 * atan2f(sqrtf(a), sqrtf(1 - a));
    float d = R * c;

    return d;
}

float bear(float phi1, float lambda1, float phi2, float lambda2)
{
    float y = sinf(lambda2 - lambda1) * cosf(phi2);
    float x = cosf(phi1) * sinf(phi2) - sinf(phi1) * cosf(phi2) * cosf(lambda2 - lambda1);
    float theta = atan2f(y, x);
    float brng = theta;

    return brng;
}

float crossarc(point_t p1, point_t p2, point_t p3)
{
    float phi1 = deg2rad(p1.lat);
    float lambda1 = deg2rad(p1.lon);
    float phi2 = deg2rad(p2.lat);
    float lambda2 = deg2rad(p2.lon);
    float phi3 = deg2rad(p3.lat);
    float lambda3 = deg2rad(p2.lon);

    float theta12 = bear(phi1, lambda1, phi2, lambda2);
    float theta13 = bear(phi1, lambda1, phi3, lambda3);
    float d13 = distance(p1, p3);

    float diff = fabsf(theta13 - theta12);
    if (diff > M_PI)
    {
        diff = 2 * M_PI - diff;
    }

    float dXa;
    if (diff > M_PI / 2)
    {
        dXa = d13;
    }
    else
    {
        float dXt = asinf(sinf(d13 / R) * sinf(theta13 - theta12)) * R;

        float d12 = distance(p1, p2);
        float d14 = acosf(cosf(d13 / R) / cosf(dXt / R)) * R;
        dXa = (d14 > d12) ? distance(p2, p3) : fabs(dXt);
    }

    return dXa;
}

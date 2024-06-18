/*
 * Filename: latlon.c
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-06-12
 * Last Modified: 2024-06-18
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

float distance(Point p1, Point p2)
{
    float φ1 = deg2rad(p1.lat);
    float φ2 = deg2rad(p2.lat);
    float Δφ = deg2rad(p2.lat - p1.lat);
    float Δλ = deg2rad(p2.lon - p1.lon);

    float a = sinf(Δφ / 2) * sinf(Δφ / 2) + cosf(φ1) * cosf(φ2) * sinf(Δλ / 2) * sinf(Δλ / 2);
    float c = 2 * atan2f(sqrtf(a), sqrtf(1 - a));
    float d = R * c;

    return d;
}

float bear(float φ1, float λ1, float φ2, float λ2)
{
    float y = sinf(λ2 - λ1) * cosf(φ2);
    float x = cosf(φ1) * sinf(φ2) - sinf(φ1) * cosf(φ2) * cosf(λ2 - λ1);
    float θ = atan2f(y, x);
    float brng = θ;

    return brng;
}

float crossarc(Point p1, Point p2, Point p3)
{
    float φ1 = deg2rad(p1.lat);
    float λ1 = deg2rad(p1.lon);
    float φ2 = deg2rad(p2.lat);
    float λ2 = deg2rad(p2.lon);
    float φ3 = deg2rad(p3.lat);
    float λ3 = deg2rad(p2.lon);

    float θ12 = bear(φ1, λ1, φ2, λ2);
    float θ13 = bear(φ1, λ1, φ3, λ3);
    float d13 = distance(p1, p3);

    float diff = fabsf(θ13 - θ12);
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
        float dXt = asinf(sinf(d13 / R) * sinf(θ13 - θ12)) * R;

        float d12 = distance(p1, p2);
        float d14 = acosf(cosf(d13 / R) / cosf(dXt / R)) * R;
        dXa = (d14 > d12) ? distance(p2, p3) : fabs(dXt);
    }

    return dXa;
}

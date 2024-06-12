/*
 * Filename: latlon.h
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

#ifndef LATLON_H
#define LATLON_H

typedef struct
{
    float lat;
    float lon;
} Point;

float deg2grad(Point p);

/*
 This uses the ‘haversine’ formula to calculate the great-circle
 distance between two points – that is, the shortest distance
 over the earth’s surface – giving an ‘as-the-crow-flies’ distance
 between the points (ignoring any hills they fly over, of course!).
 Input lat1, lon1, lat1, lon2 in degrees.
*/
float distance(Point p1, Point p2);

float bear(float φ1, float λ1, float φ2, float λ2);

/*
 Calculates the shortest distance in meters between an arc
 (defined by p1 and p2) and a third point, p3.
 Input lat1, lon1, lat2, lon2, lat3, lon3 in degrees.
*/
float crossarc(Point p1, Point p2, Point p3);

#endif

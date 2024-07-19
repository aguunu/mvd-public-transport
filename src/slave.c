/*
 * Filename: slave.c
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-06-18
 * Last Modified: 2024-07-19
 *
 * License: See LICENSE file in the project root for license information.
 */

#include "slave.h"
#include "bus-record.h"
#include "critical-points.h"
#include "latlon.h"
#include "model.h"
#include "section.h"
#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void section_analysis(work_t *data, variant_t *info)
{
    int n_stops = info->n_sections + 1;

    // float stop_arrival_time[n_stops];
    float err[n_stops];
    record_t nearest_data[n_stops];
    for (int i = 0; i < info->n_sections; i++)
    {
        point_t stop = info->sections[i]->path[0];
        err[i] = -1;

        for (int j = 0; j < data->n; j++)
        {
            record_t record = data->records[j];

            float d = distance(stop, record.p);

            if (d < err[i] || err[i] == -1)
            {
                err[i] = d;
                // stop_arrival_time[i] = record.timestamp;
                nearest_data[i] = record;
            }
        }
    }

    // Section velocity analysis
    float avg_speed[n_stops];
    for (int i = 0; i < (n_stops - 1); i++)
    {
        if (err[i] > RADIUS_THRESHOLD_M || err[i + 1] > RADIUS_THRESHOLD_M)
        {
            avg_speed[i] = 0;
            continue;
        }

        record_t data1 = nearest_data[i];
        record_t data2 = nearest_data[i + 1];

        float delta_x = section_length(info->sections[i]);
        time_t delta_t = data2.timestamp - data1.timestamp;

        // TODO
        if (delta_t <= 0)
        {
            continue;
        }

        avg_speed[i] = (delta_x / delta_t) * 3.6;
        unsigned int round_speed = (unsigned int)avg_speed[i];

        if (round_speed < HISTOGRAM_BINS)
        {
            time_t montevideo_time = data1.timestamp - 3600 * 3; // Montevideo time zone UTC-3
            struct tm *timeinfo;
            timeinfo = gmtime(&montevideo_time);

            int hh = timeinfo->tm_hour;
            assert(0 <= hh && hh < 24);

#pragma omp atomic
            info->sections[i]->metrics.histogram[hh][round_speed]++;
        }
    }
}

void instant_speed_analysis(work_t *buf, crit_points_t *critical_points)
{
    float instant_speed[buf->n - 1];
    for (int i = 0; i < buf->n - 1; i++)
    {
        record_t data1 = buf->records[i];
        record_t data2 = buf->records[i + 1];

        float delta_d = distance(data1.p, data2.p);
        float delta_t = (float)(data2.timestamp - data1.timestamp);

        assert(delta_d >= 0);
        assert(delta_t >= 0);
        if (delta_t < 0 || delta_t > 60)
        {
            continue;
        }

        instant_speed[i] = (delta_d / delta_t) * 3.6;
        assert(instant_speed[i] >= 0);

        if (instant_speed[i] > SPEED_LIMIT_KM_H)
        {
            // TODO
        }
        for (int j = 0; j < critical_points->n; j++)
        {
            crit_point_t *cp = &critical_points->p[j];
            float d = crossarc(data1.p, data2.p, cp->p);
            if (d < RADIUS_THRESHOLD_M && instant_speed[i] > SPEED_LIMIT_KM_H)
            {
                int round_speed = (unsigned int)instant_speed[i];
                if (round_speed < HISTOGRAM_BINS)
                {
                    assert(round_speed >= 0);
                    time_t montevideo_time = data1.timestamp - 3600 * 3; // Montevideo time zone UTC-3
                    struct tm *timeinfo;
                    timeinfo = gmtime(&montevideo_time);

                    int hh = timeinfo->tm_hour;
                    assert(0 <= hh && hh < 24);

#pragma omp atomic
                    cp->metrics.histogram[hh][round_speed]++;
                }
            }
        }
    }
}

void do_work(work_t *buf, variant_t *info, crit_points_t *critical_points)
{
    assert(buf->n > 0);
    assert(buf->n < MAX_DATA_PER_TRIP);

    if (info)
    {
        assert(info->sections != NULL);
        assert(info->n_sections > 0);
        section_analysis(buf, info);
    }

    // instant_speed_analysis(buf, critical_points);

    free(buf);
}

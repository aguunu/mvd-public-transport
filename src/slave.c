/*
 * Filename: slave.c
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-06-18
 * Last Modified: 2024-07-22
 *
 * License: See LICENSE file in the project root for license information.
 */

#include "slave.h"
#include "critical-points.h"
#include "latlon.h"
#include "model.h"
#include "section.h"
#include "work.h"
#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void section_analysis(work_t *data, model_t *model)
{
    variant_t *info = model->variants[data->variant];
    assert(info->sections != NULL);
    assert(info->n_sections > 0);

    int n_stops = info->n_sections + 1;

    // calculate memory required
    size_t total_size = (sizeof(float) * n_stops) +
                        (sizeof(mini_record_t) * n_stops) +
                        (sizeof(float) * n_stops);

    // allocate block of memory
    void *block = malloc(total_size);
    assert(block != NULL);

    float *err = (float *)block;
    mini_record_t *nearest_data = (mini_record_t *)(err + n_stops);
    float *avg_speed = (float *)(nearest_data + n_stops);

    // compute nearest data
    for (int i = 0; i < info->n_sections; i++)
    {
        point_t stop = info->sections[i]->path[0];
        err[i] = -1;

        for (int j = 0; j < data->n; j++)
        {
            mini_record_t record = data->records[j];

            float d = distance(stop, record.p);

            if (d < err[i] || err[i] == -1)
            {
                err[i] = d;
                nearest_data[i] = record;
            }
        }
    }

    // section velocity analysis
    for (int i = 0; i < (n_stops - 1); i++)
    {
        if (err[i] > RADIUS_THRESHOLD_M || err[i + 1] > RADIUS_THRESHOLD_M)
        {
            avg_speed[i] = 0;
            continue;
        }

        mini_record_t data1 = nearest_data[i];
        mini_record_t data2 = nearest_data[i + 1];

        float delta_x = info->sections[i]->length;
        time_t delta_t = data2.timestamp - data1.timestamp;

        if (delta_t <= 0)
        {
            avg_speed[i] = 0;
            continue;
        }
        assert(delta_t > 0);
        assert(delta_x > 0);

        avg_speed[i] = (delta_x / delta_t) * 3.6;
        int round_speed = (int)avg_speed[i];

        assert(round_speed >= 0);

        if (round_speed < HISTOGRAM_BINS)
        {
            assert(data1.timestamp >= model->config->from_t);
            assert(data1.timestamp <= model->config->to_t);

            int seconds_per_hour = 60 * 60;
            int histogram_idx = (data1.timestamp - model->config->from_t) / (seconds_per_hour);

#pragma omp atomic
            info->sections[i]->metrics[histogram_idx].histogram[round_speed]++;
        }
    }

    free(block);
}

void instant_speed_analysis(work_t *buf, crit_points_t *critical_points)
{
    float *instant_speed = malloc(sizeof(float) * (buf->n - 1));

    for (int i = 0; i < buf->n - 1; i++)
    {
        mini_record_t data1 = buf->records[i];
        mini_record_t data2 = buf->records[i + 1];

        float delta_d = distance(data1.p, data2.p);
        float delta_t = (float)(data2.timestamp - data1.timestamp);

        assert(delta_d >= 0);
        assert(delta_t >= 0);
        if (delta_t <= 0 || delta_t > 60)
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

                    // #pragma omp atomic
                    //                     cp->metrics.histogram[hh][round_speed]++;
                }
            }
        }
    }

    free(instant_speed);
}

void slave(work_t *buf, model_t *model)
{
    assert(buf != NULL);
    assert(buf->n > 0);
    assert(buf->n <= MAX_DATA_PER_TRIP);

    int vcode = buf->variant;
    if (model->variants[vcode])
    {
        section_analysis(buf, model);
    }

    // instant_speed_analysis(buf, critical_points);

    free(buf);
}

/*
 * Filename: slave.c
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-06-18
 * Last Modified: 2024-07-27
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

#pragma omp atomic update
            info->sections[i]->metrics[histogram_idx].histogram[round_speed]++;
        }
    }

    free(block);
}

void instant_speed_analysis_v2(work_t *buf, model_t *model)
{
    crit_points_t *cps = &model->critical_points;
    for (int i = 0; i < cps->n; i++)
    {
        crit_point_t *p = &cps->p[i];

        int j = 0;
        while (j < buf->n - 1)
        {
            float d = 0;
            float t = 0;

            while (j < buf->n - 1 && crossarc(buf->records[j].p, buf->records[j + 1].p, p->p) < RADIUS_THRESHOLD_M)
            {
                mini_record_t *data1 = &buf->records[j];
                mini_record_t *data2 = &buf->records[j + 1];

                float delta_d = distance(data2->p, data1->p);
                float delta_t = (float)(data2->timestamp - data1->timestamp);
                if (delta_t <= 0 || delta_t > 60)
                {
                    j++;
                    continue;
                }

                assert(delta_d >= 0);
                assert(delta_t >= 0);

                d += delta_d;
                t += delta_t;

                j++;
            }

            // if not near
            if (d == 0 || t == 0)
            {
                j++;
            }
            else
            {
                float instant_speed = (d / t) * 3.6;
                assert(instant_speed >= 0);

                int round_speed = (int)instant_speed;
                assert(round_speed >= 0);

                int data_idx = j - 1;
                assert(0 <= data_idx);
                assert(data_idx < buf->n);
                time_t t = buf->records[data_idx].timestamp;

                if (round_speed < HISTOGRAM_BINS)
                {
                    assert(t >= model->config->from_t);
                    assert(t <= model->config->to_t);

                    int seconds_per_hour = 60 * 60;
                    int histogram_idx = (t - model->config->from_t) / (seconds_per_hour);

#pragma omp atomic update
                    p->metrics[histogram_idx].histogram[round_speed]++;
                }
            }
        }
    }
}

void instant_speed_analysis(work_t *buf, model_t *model)
{
    crit_points_t *cps = &model->critical_points;

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

        float instant_speed = (delta_d / delta_t) * 3.6;
        assert(instant_speed >= 0);

        for (int j = 0; j < cps->n; j++)
        {
            crit_point_t *cp = &cps->p[j];
            float d = crossarc(data1.p, data2.p, cp->p);
            if (d < RADIUS_THRESHOLD_M)
            {
                int round_speed = (int)instant_speed;
                assert(round_speed >= 0);

                if (round_speed < HISTOGRAM_BINS)
                {
                    assert(data1.timestamp >= model->config->from_t);
                    assert(data1.timestamp <= model->config->to_t);

                    int seconds_per_interval = 15 * 60;
                    int delta_t = data1.timestamp - model->config->from_t;
                    int interval = delta_t / seconds_per_interval;
                    assert(interval >= 0);
#pragma omp atomic
                    cp->metrics[interval].histogram[round_speed]++;
                }
            }
        }
    }
}

void slave(work_t *buf, model_t *model)
{
    assert(buf != NULL);
    assert(buf->n > 0);
    assert(buf->n <= MAX_DATA_PER_TRIP);

    if (model->variants[buf->variant] != NULL)
    {
        section_analysis(buf, model);
    }

    instant_speed_analysis_v2(buf, model);

    free(buf);
}

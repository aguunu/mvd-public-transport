/*
 * Filename: model.c
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

#include "model.h"
#include "colors.h"
#include "config.h"
#include "latlon.h"
#include "section.h"
#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void load_points(model_t *model, char *file_path)
{
    FILE *fp = fopen(file_path, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Fail to open %s.\n", file_path);
    }

    crit_points_t *cps = &model->critical_points;

    int alloc_size = 256;

    cps->p = (crit_point_t *)malloc(sizeof(crit_point_t) * alloc_size);
    cps->n = 0;

    while (1)
    {
        if (cps->n == alloc_size)
        {
            alloc_size *= 2;
            cps->p = (crit_point_t *)realloc(cps->p, sizeof(crit_point_t) * alloc_size);
        }

        crit_point_t *cp = &cps->p[cps->n];
        int got = fscanf(fp, "%f,%f,%d",
                         &cp->p.lat,
                         &cp->p.lon,
                         &cp->type);
        if (got != 3)
        {
            break;
        }
        omp_init_lock(&cp->lock);
        for (int hh = 0; hh < 24; hh++)
        {
            for (int i = 0; i < HISTOGRAM_BINS; i++)
            {
                cp->metrics.histogram[i] = 0;
            }
        }
        cps->n++;
    }

    fclose(fp);

    fprintf(stdout, GREEN "+ Added %d critical points.\n", cps->n);
}

void destroy_points(crit_points_t *cps)
{
    for (int i = 0; i < cps->n; i++)
    {
        omp_destroy_lock(&cps->p[i].lock);
    }

    free(cps->p);
}

void load_sections_info(model_t *model, char *file_path)
{
    FILE *fp = fopen(file_path, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Fail to open %s.\n", file_path);
    }

    model->sections = (section_t **)calloc(MAX_SECTIONS, sizeof(section_t *));
    section_t **sections = model->sections;

    char *line = NULL;
    size_t len = 0;

    int s_code = 0;

    int delta_t = model->config->to_t - model->config->from_t;
    assert(delta_t > 0);
    int total_hh = delta_t / (60 * 60);

    while (getline(&line, &len, fp) != -1)
    {
        assert(s_code < MAX_SECTIONS);
        assert(sections[s_code] == NULL);
        int arr_size = 256;

        sections[s_code] = calloc(1, sizeof(section_t));
        section_t *section = sections[s_code];

        section->path = (point_t *)malloc(sizeof(point_t) * arr_size);
        section->n_path = 0;
        section->metrics = calloc(total_hh, sizeof(metrics_t));
        // omp_init_lock(&section->metrics.lock);
        char *token = strtok(line, ";");
        while (token)
        {
            point_t p;
            sscanf(token, "[%f,%f]", &p.lon, &p.lat);
            section->id = s_code;
            section->path[section->n_path] = p;
            section->n_path++;
            if (section->n_path == arr_size)
            {
                arr_size *= 2;
                section->path = (point_t *)realloc(section->path, sizeof(point_t) * arr_size);
            }
            token = strtok(NULL, ";");
        }
        section->length = section_length(section);
        assert(section->length > 0);
        s_code++;
    }
    free(line);

    fclose(fp);

    printf(GREEN "+ Added %d sections.\n" NO_COLOR, s_code);
}

void load_variants_info(model_t *model, char *file_path)
{
    FILE *fp = fopen(file_path, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Fail to open %s.\n", file_path);
    }

    model->variants = (variant_t **)calloc(MAX_VARIANT, sizeof(variant_t *));
    section_t **sections = model->sections;
    variant_t **vs = model->variants;

    char *line = NULL;
    size_t len = 0;

    int count = 0;
    while (getline(&line, &len, fp) != -1)
    {
        count++;
        char *token = strtok(line, ",");
        int v_code = atoi(token);

        vs[v_code] = (variant_t *)malloc(sizeof(variant_t));
        variant_t *v = vs[v_code];

        // get new token
        token = strtok(NULL, ",");
        v->n_sections = 0;
        while (token)
        {
            int section_idx = atoi(token);
            v->sections[v->n_sections] = sections[section_idx];
            v->n_sections++;
            token = strtok(NULL, ",");
        };
    }
    free(line);
    fclose(fp);

    printf(GREEN "+ Added %d variants.\n" NO_COLOR, count);
}

void destroy_sections(model_t *model)
{
    for (int i = 0; i < MAX_VARIANT; i++)
    {
        if (model->variants[i])
        {
            free(model->variants[i]);
        }
    }
    free(model->variants);

    for (int i = 0; i < MAX_SECTIONS; i++)
    {
        if (model->sections[i])
        {
            // omp_destroy_lock(&model->sections[i]->metrics.lock);
            free(model->sections[i]->path);
            free(model->sections[i]->metrics);
            free(model->sections[i]);
        }
    }
    free(model->sections);
}

int load_model(model_t *model, config_t *config)
{
    model->config = config;

    // load critical points files
    load_points(model, config->input_points);

    // load section info
    load_sections_info(model, config->input_sections);

    // load variant info
    load_variants_info(model, config->input_variants);

    return 0;
}

void destroy_model(model_t *model)
{
    destroy_points(&model->critical_points);
    destroy_sections(model);
}

void save_model(model_t *model)
{
    FILE *fp = fopen(model->config->output_results, "w");
    if (fp == NULL)
    {
        fprintf(stdout, "Failed to open %s.", model->config->output_results);
    }

    time_t delta_t = model->config->to_t - model->config->from_t;
    assert(delta_t > 0);
    int total_hh = delta_t / (60 * 60);
    printf("%d\n", total_hh);

    char date_buf[128];
    for (int hh_idx = 0; hh_idx < total_hh; hh_idx++)
    {
        time_t timestamp = model->config->from_t + hh_idx * 60 * 60;
        time_t montevideo_time = timestamp - 3600 * 3; // Montevideo time zone UTC-3

        struct tm *timeinfo;
        timeinfo = gmtime(&montevideo_time);

        strftime(date_buf, sizeof(date_buf), "%Y-%m-%dT%H:%M:%S", timeinfo);

        for (int section_id = 0; section_id < MAX_SECTIONS; section_id++)
        {
            section_t *s = model->sections[section_id];
            if (!s)
            {
                continue;
            }

            fprintf(fp, "%s,%d,", date_buf, section_id);
            for (int bin = 0; bin < HISTOGRAM_BINS; bin++)
            {
                int count = s->metrics[hh_idx].histogram[bin];
                fprintf(fp, "%d;", count);
            }
            fprintf(fp, "\n");
        }
    }

    fclose(fp);

    fprintf(stdout, MAGENTA "Results saved in \"%s\"\n" NO_COLOR, model->config->output_results);
}

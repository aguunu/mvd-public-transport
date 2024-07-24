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
#include "critical-points.h"
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
                         &cp->p.lon,
                         &cp->p.lat,
                         &cp->type);
        if (got != 3)
        {
            break;
        }

        time_t delta_t = model->config->to_t - model->config->from_t;
        assert(delta_t > 0);

        int intervals = delta_t / model->config->interval;
        cp->metrics = calloc(intervals, sizeof(metrics_t));
        cps->n++;
    }

    fclose(fp);

    fprintf(stdout, GREEN "+ Added %d critical points.\n", cps->n);
}

void destroy_points(crit_points_t *cps)
{
    for (int i = 0; i < cps->n; i++)
    {
        free(cps->p[i].metrics);
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

    size_t nmemb_sections = 1024;
    model->sections.sections = calloc(nmemb_sections, sizeof(section_t));
    model->sections.n = 0;

    // model->sections = (section_t **)calloc(MAX_SECTIONS, sizeof(section_t *));
    // section_t **sections = model->sections;

    char *line = NULL;
    size_t len = 0;

    // int s_code = 0;

    int delta_t = model->config->to_t - model->config->from_t;
    assert(delta_t > 0);
    int total_intervals = delta_t / model->config->interval;

    while (getline(&line, &len, fp) != -1)
    {
        if ((size_t)model->sections.n == nmemb_sections)
        {
            nmemb_sections *= 2;
            model->sections.sections = realloc(model->sections.sections, nmemb_sections * sizeof(section_t));
        }

        int s_code = model->sections.n;
        // assert(s_code < MAX_SECTIONS);
        // assert(sections[s_code] == NULL);
        size_t nmemb_path = 256;

        // sections[s_code] = calloc(1, sizeof(section_t));
        // section_t *section = sections[s_code];
        section_t *section = &model->sections.sections[s_code];
        section->path = (point_t *)malloc(sizeof(point_t) * nmemb_path);
        section->n_path = 0;
        section->metrics = calloc(total_intervals, sizeof(metrics_t));
        // omp_init_lock(&section->metrics.lock);
        char *token = strtok(line, ";");
        while (token)
        {
            point_t p;
            sscanf(token, "[%f,%f]", &p.lon, &p.lat);
            section->id = s_code;
            section->path[section->n_path] = p;
            section->n_path++;
            if ((size_t)section->n_path == nmemb_path)
            {
                nmemb_path *= 2;
                section->path = (point_t *)realloc(section->path, sizeof(point_t) * nmemb_path);
            }
            token = strtok(NULL, ";");
        }
        section->length = section_length(section);
        assert(section->length > 0);
        s_code++;
        model->sections.n++;
    }
    free(line);

    fclose(fp);

    printf(GREEN "+ Added %d sections.\n" NO_COLOR, model->sections.n);
}

void load_variants_info(model_t *model, char *file_path)
{
    FILE *fp = fopen(file_path, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Fail to open %s.\n", file_path);
    }

    model->variants = calloc(MAX_VARIANT, sizeof(variant_t *));
    sections_t *sections = &model->sections;
    variant_t **variants = model->variants;

    // section_t **sections = model->sections;
    // variant_t **vs = model->variants;

    char *line = NULL;
    size_t len = 0;

    int count = 0;

    while (getline(&line, &len, fp) != -1)
    {
        count++;
        char *token = strtok(line, ",");
        int v_code = atoi(token);

        // vs[v_code] = (variant_t *)malloc(sizeof(variant_t));
        // variant_t *v = vs[v_code];
        variants[v_code] = malloc(sizeof(variant_t));
        variant_t *v = variants[v_code];
        assert(v != NULL);

        size_t nmemb_sections = 16;
        v->sections = calloc(nmemb_sections, sizeof(section_t *));
        v->n_sections = 0;

        // get new token
        token = strtok(NULL, ",");
        v->n_sections = 0;
        while (token)
        {
            if ((size_t)v->n_sections == nmemb_sections)
            {
                nmemb_sections *= 2;
                v->sections = realloc(v->sections, nmemb_sections * sizeof(section_t *));
            }
            int section_idx = atoi(token);
            v->sections[v->n_sections] = &sections->sections[section_idx];
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

    free(model->sections.sections);
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

void save_critical_points(model_t *model)
{
    FILE *fp = fopen(model->config->output_points, "w");
    if (fp == NULL)
    {
        fprintf(stdout, "Failed to open %s.", model->config->output_points);
    }

    time_t delta_t = model->config->to_t - model->config->from_t;
    assert(delta_t > 0);
    int total_intervals = delta_t / model->config->interval;

    char date_buf[128];
    for (int interval = 0; interval < total_intervals; interval++)
    {
        time_t timestamp = model->config->from_t + interval * model->config->interval;
        time_t montevideo_time = timestamp - 3600 * 3; // Montevideo time zone UTC-3

        struct tm *timeinfo;
        timeinfo = gmtime(&montevideo_time);

        strftime(date_buf, sizeof(date_buf), "%Y-%m-%dT%H:%M:%S", timeinfo);

        for (int idx = 0; idx < model->critical_points.n; idx++)
        {
            crit_point_t *p = &model->critical_points.p[idx];
            assert(p != NULL);

            fprintf(fp, "%s,%d,", date_buf, idx);
            for (int bin = 0; bin < HISTOGRAM_BINS; bin++)
            {
                int count = p->metrics[interval].histogram[bin];
                fprintf(fp, "%d;", count);
            }
            fprintf(fp, "\n");
        }
    }

    fclose(fp);

    fprintf(stdout, MAGENTA "Results saved in \"%s\"\n" NO_COLOR, model->config->output_points);
}

void save_sections(model_t *model)
{
    FILE *fp = fopen(model->config->output_sections, "w");
    if (fp == NULL)
    {
        fprintf(stdout, "Failed to open %s.", model->config->output_sections);
    }

    time_t delta_t = model->config->to_t - model->config->from_t;
    assert(delta_t > 0);
    int total_intervals = delta_t / model->config->interval;

    char date_buf[128];
    for (int interval = 0; interval < total_intervals; interval++)
    {
        time_t timestamp = model->config->from_t + interval * model->config->interval;
        time_t montevideo_time = timestamp - 3600 * 3; // Montevideo time zone UTC-3

        struct tm *timeinfo;
        timeinfo = gmtime(&montevideo_time);

        strftime(date_buf, sizeof(date_buf), "%Y-%m-%dT%H:%M:%S", timeinfo);

        for (int section_id = 0; section_id < model->sections.n; section_id++)
        {
            section_t *s = &model->sections.sections[section_id];
            // if (!s)
            // {
            //     continue;
            // }

            fprintf(fp, "%s,%d,", date_buf, section_id);
            for (int bin = 0; bin < HISTOGRAM_BINS; bin++)
            {
                int count = s->metrics[interval].histogram[bin];
                fprintf(fp, "%d;", count);
            }
            fprintf(fp, "\n");
        }
    }

    fclose(fp);

    fprintf(stdout, MAGENTA "Results saved in \"%s\"\n" NO_COLOR, model->config->output_sections);
}

void save_model(model_t *model)
{
    save_sections(model);
    save_critical_points(model);
}

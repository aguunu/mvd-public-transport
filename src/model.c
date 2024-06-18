/*
 * Filename: model.c
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-06-18
 * Last Modified: 2024-06-18
 *
 * License: See LICENSE file in the project root for license information.
 */

#include "model.h"
#include "colors.h"
#include "critical-points.h"
#include "latlon.h"
#include "section.h"
#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void load_critical_points(CriticalPoints *cps)
{
    char *file_path = "../critical-points.csv";

    int alloc_size = 256;

    FILE *fp = fopen(file_path, "r");

    cps->p = (CriticalPoint *)malloc(sizeof(CriticalPoint) * alloc_size);
    cps->n = 0;

    while (1)
    {
        if (cps->n == alloc_size)
        {
            alloc_size *= 2;
            cps->p = (CriticalPoint *)realloc(cps->p, sizeof(CriticalPoint) * alloc_size);
        }

        CriticalPoint *cp = &cps->p[cps->n];
        int got = fscanf(fp, "%f,%f,%d",
                         &cp->p.lat,
                         &cp->p.lon,
                         &cp->type);
        if (got != 3)
        {
            break;
        }
        omp_init_lock(&cp->lock);
        for (int i = 0; i < HISTOGRAM_BINS; i++)
        {
            cp->metrics.histogram[i] = 0;
        }
        cps->n++;
    }

    fclose(fp);

    fprintf(stdout, GREEN "+ Added %d critical points.\n", cps->n);
}

void destroy_critical_points(CriticalPoints *cps)
{
    for (int i = 0; i < cps->n; i++)
    {
        omp_destroy_lock(&cps->p[i].lock);
    }

    free(cps->p);
}

void read_sections(Section **sections, FILE *fp)
{
    char *line = NULL;
    size_t len = 0;

    int s_code = 0;
    while (getline(&line, &len, fp) != -1)
    {
        assert(s_code < MAX_SECTIONS);
        assert(sections[s_code] == NULL);
        int arr_size = 256;

        sections[s_code] = calloc(1, sizeof(Section));
        Section *section = sections[s_code];

        section->path = (Point *)malloc(sizeof(Point) * arr_size);
        section->n_path = 0;
        omp_init_lock(&section->metrics.lock);
        char *token = strtok(line, ";");
        while (token)
        {
            Point p;
            sscanf(token, "[%f,%f]", &p.lon, &p.lat);
            section->id = s_code;
            section->path[section->n_path] = p;
            section->n_path++;
            if (section->n_path == arr_size)
            {
                arr_size *= 2;
                section->path = (Point *)realloc(section->path, sizeof(Point) * arr_size);
            }
            token = strtok(NULL, ";");
        }
        s_code++;
    }
    free(line);

    printf(GREEN "+ Added %d sections.\n" NO_COLOR, s_code);
}

void read_variant_info(Section **sections, VariantInfo **vs, FILE *fp)
{
    char *line = NULL;
    size_t len = 0;

    int count = 0;
    while (getline(&line, &len, fp) != -1)
    {
        count++;
        char *token = strtok(line, ",");
        int v_code = atoi(token);

        vs[v_code] = (VariantInfo *)malloc(sizeof(VariantInfo));
        VariantInfo *v = vs[v_code];

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

    printf(GREEN "+ Added %d variants.\n" NO_COLOR, count);
}

void read_sections_variants(model_t *model)
{
    // Read section info
    FILE *fp_sections = fopen("../sections.txt", "r");
    model->sections = (Section **)calloc(MAX_SECTIONS, sizeof(Section *));
    read_sections(model->sections, fp_sections);
    fclose(fp_sections);

    // Read variant sections info
    FILE *fp_vs = fopen("../variant-sections.txt", "r");
    model->variants = (VariantInfo **)calloc(MAX_VARIANT, sizeof(VariantInfo *));
    read_variant_info(model->sections, model->variants, fp_vs);
    fclose(fp_vs);
}

void destroy_sections_variants(model_t *model)
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
            omp_destroy_lock(&model->sections[i]->metrics.lock);
            free(model->sections[i]->path);
            free(model->sections[i]);
        }
    }
    free(model->sections);
}

void load_model(model_t *model)
{
    load_critical_points(&model->critical_points);
    read_sections_variants(model);
}

void destroy_model(model_t *model)
{
    destroy_critical_points(&model->critical_points);
    destroy_sections_variants(model);
}

void save_model(model_t *model)
{
    char *file_path = "../sections-results.csv";

    FILE *fp = fopen(file_path, "w");
    if (fp == NULL)
    {
        fprintf(stdout, RED "Failed to open %s." NO_COLOR, file_path);
    }

    for (int i = 0; i < MAX_SECTIONS; i++)
    {
        Section *s = model->sections[i];
        if (!s)
        {
            continue;
        }

        fprintf(fp, "%d,", i);
        fprintf(fp, "[");
        for (int j = 0; j < HISTOGRAM_BINS; j++)
        {
            fprintf(fp, "%d,", s->metrics.histogram[j]);
        }
        fprintf(fp, "]\n");
    }
    fclose(fp);
}

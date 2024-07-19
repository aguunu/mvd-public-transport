/*
 * Filename: main.c
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-06-12
 * Last Modified: 2024-07-18
 *
 * License: See LICENSE file in the project root for license information.
 */

#include "args.h"
#include "colors.h"
#include "config.h"
#include "master.h"
#include "model.h"
#include "reader.h"
#include <omp.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    options_t options;
    options_parser(argc, argv, &options);

#ifdef DEBUG
    fprintf(stdout, BLUE "Command line options:\n" NO_COLOR);
    fprintf(stdout, BROWN "help: %d\n" NO_COLOR, options.help);
    fprintf(stdout, BROWN "version: %d\n" NO_COLOR, options.version);
    fprintf(stdout, BROWN "use colors: %d\n" NO_COLOR, options.use_colors);
    fprintf(stdout, BROWN "filename: %s\n" NO_COLOR, options.file_name);
    fprintf(stdout, BROWN "n-threads: %d\n" NO_COLOR, options.n_threads);
#endif
    config_t config;
    config_init(&config, options.file_name);

    omp_set_num_threads(options.n_threads);

    model_t model;
    load_model(&model, &config);

    reader_t reader;
    int err = reader_init(&reader, &config);

    master(&reader, &model);

    reader_destroy(&reader);
    save_model(&model);
    destroy_model(&model);
    config_destroy(&config);

    return EXIT_SUCCESS;
}

/*
 * Filename: args.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "colors.h"
#include "messages.h"

/*
 * Sets the default options
 */
static void
set_default_options(options_t *options)
{
    options->help = false;
    options->version = false;
    options->use_colors = true;
    options->n_threads = 1;
}

/*
 * Finds the matching case of the current command line option
 */
void switch_options(int arg, options_t *options)
{
    switch (arg)
    {
    case 'h':
        options->help = true;
        help();
        exit(EXIT_SUCCESS);

    case 'v':
        options->version = true;
        version();
        exit(EXIT_SUCCESS);

    case 0:
        options->use_colors = false;
        break;

    case 'n':
        options->n_threads = atoi(optarg);
        break;

    case '?':
        usage();
        exit(EXIT_FAILURE);

    default:
        usage();
        abort();
    }
}

/*
 * Tries to get the file name. Otherwise, gets stdin
 */
void get_file_name(int argc, char *argv[], options_t *options)
{

    /* If there is more arguments, probably, it is an input file */
    if (optind < argc)
    {
        strncpy(options->file_name, argv[optind++], FILE_NAME_SIZE);

        /* Otherwise, assumes stdin as the input file */
    }
    else
    {
        strncpy(options->file_name, "-", FILE_NAME_SIZE);
    }
}

/*
 * Public function that loops until command line options were parsed
 */
void options_parser(int argc, char *argv[], options_t *options)
{
    set_default_options(options);

    int arg; /* Current option */

    /* getopt allowed options */
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {"no-colors", no_argument, 0, 0},
        {"threads", required_argument, 0, 'n'},
    };

    while (true)
    {

        int option_index = 0;
        arg = getopt_long(argc, argv, "hvtn:", long_options, &option_index);

        /* End of the options? */
        if (arg == -1)
            break;

        /* Find the matching case of the argument */
        switch_options(arg, options);
    }

    /* Gets the file name or exits with error */
    get_file_name(argc, argv, options);
}

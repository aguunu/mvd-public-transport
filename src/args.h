/*
 * Filename: args.h
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

#ifndef ARGS_H
#define ARGS_H

#include <getopt.h>
#include <stdbool.h>

/* Max size of a file name */
#define FILE_NAME_SIZE 512

/* Defines the command line allowed options struct */
struct options
{
    bool help;
    bool version;
    bool use_colors;
    char file_name[FILE_NAME_SIZE];
    int n_threads;
};

/* Exports options as a global type */
typedef struct options options_t;

/* Public functions section */
void options_parser(int argc, char *argv[], options_t *options);

#endif

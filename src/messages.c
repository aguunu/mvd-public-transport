/*
 * Filename: messages.c
 *
 * Author(s):
 *     Agustín Núñez <agustin.nunez@fing.edu.uy>
 *     Paula Abbona <paula.abbona@fing.edu.uy>
 *
 * Creation Date: 2024-06-12
 * Last Modified: 2024-08-17
 *
 * License: See LICENSE file in the project root for license information.
 */

#include <stdio.h>
#include <stdlib.h>

#include "colors.h"
#include "messages.h"

/*
 * Help message
 */
void help()
{
    fprintf(stdout, BLUE __PROGRAM_NAME__ "\n\n" NO_COLOR);
    usage();
    description();
    options();
    author();
    version();
}

/*
 * Usage message
 */
void usage()
{
    fprintf(stdout, BROWN "Usage: " NO_COLOR);
    fprintf(stdout, "%s [options] input file\n\n", __PROGRAM_NAME__);
}

/*
 * Description message
 */
void description()
{
    fprintf(stdout, BROWN "Description: " NO_COLOR);
    fprintf(stdout,
            "Model for estimating public transport speed metrics "
            "across different sections of Montevideo city.\n");
}

/*
 * Options message
 */
void options()
{
    fprintf(stdout, BROWN "Options:\n\n" NO_COLOR);
    fprintf(stdout, GRAY "\t-v | --version\n" NO_COLOR "\t\tPrints %s version.\n\n", __PROGRAM_NAME__);
    fprintf(stdout, GRAY "\t-h | --help\n" NO_COLOR "\t\tPrints a help message.\n\n");
    fprintf(stdout, GRAY "\t--no-colors\n" NO_COLOR "\t\tDoes not use colors for printing.\n\n");
    fprintf(stdout, GRAY "\t--threads\n" NO_COLOR "\t\tNumber of threads to use by the program.\n\n");
}

/*
 * Author message
 */
void author()
{
    fprintf(stdout, BROWN "Written by: " GRAY "%s\n\n" NO_COLOR, __PROGRAM_AUTHOR__);
}

/*
 * Version message
 */
void version()
{
    fprintf(stdout, __PROGRAM_NAME__ " version: " GRAY "%s\n" NO_COLOR, __PROGRAM_VERSION__);
}

/***************************************************************************
 *   mv.c - move (rename) files                                            *
 *                                                                         *
 *   Copyright (C) 2014 by Darren Kirby                                    *
 *   bulliver@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#define APPNAME "mv"
#include "common.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


void show_help(void) {
    printf("Usage: %s [OPTION]... SOURCE DEST\n\
    r: %s [OPTION]... SOURCE... DIRECTORY\n\n\
    Rename SOURCE to DEST, or move SOURCE(s) to DIRECTORY.\n\n\
Options:\n\
    -f, --force\t\tdo not prompt before overwriting\n\
    -i, --interactive\tprompt before overwrite\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME, APPNAME);
}

struct optstruct {
    int force;
    int interactive;
    int verbose;
} opts;

int prompt(char *to) {
    printf("%s: %s exists. Overwrite ('y' or 'n')? ", APPNAME, to);
    char response;
    do {
        response = getchar();
    }
    while (response == '\n');

    if (response == 'y' || response == 'Y') {
        return 1;
    } else {
        return 0;
    }
}

int main(int argc, char *argv[]) {
    int opt;

    struct option longopts[] = {
        {"force", 0, NULL, 'f'},
        {"interactive", 0, NULL, 'i'},
        {"verbose", 0, NULL, 'v'},
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "fivVh", longopts, NULL)) != -1) {
        switch(opt) {
            case 'v':
                opts.verbose = 1;
                break;
            case 'i':
                opts.interactive = 1;
                break;
            case 'f':
                opts.force = 1;
                break;
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                exit(EXIT_SUCCESS);
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
            default:
                show_help();
                exit(EXIT_FAILURE);
        }
    }

    int n_args = argc - optind; /* Number of arguments */
    int isdir = 0;              /* Final arg a directory? */
    char *from;                 /* From name */
    char *to;                   /* To name */
    char tmp[256];              /* Tmp name for 'to' when it is a directory */

    struct stat s;

    if (n_args < 2) {
        g_error("at least two arguments required\n");
        show_help();
        exit(EXIT_FAILURE);
    }

    from = argv[optind];
    if (n_args == 2) {  /* A simple rename... */
        to = argv[optind+1];
        n_args = 1;     /* This ensures we traverse the do/while loop only once */
    } else {            /* mv files into directory */
        to = argv[argc-1];
        if (stat(to, &s) != 0) {
            f_error(to, "Cannot stat:");
            exit(EXIT_FAILURE);
        }
        if (!S_ISDIR(s.st_mode)) {
            g_error("Last argument must be a directory");
            exit(EXIT_FAILURE);
        } else {
            isdir = 1;
        }
        n_args--;
    }

    do {
        if (isdir) {
            snprintf(tmp, 256, "%s/%s", to, from); /* Rename 'to' */
            if (opts.interactive && access(to, F_OK) == 0) {
                if (!prompt(to)) {
                    exit(EXIT_FAILURE);
                }
            }
            if (rename(from, tmp) != 0) {
                f_error(to, NULL);
            }
        } else {
            if (opts.interactive && access(to, F_OK) == 0) {
                if (!prompt(to)) {
                    exit(EXIT_FAILURE);
                }
            }
            if (rename(from, to) != 0) {
                f_error(to, NULL);
            }
        }
        if (opts.verbose) {
            printf("'%s' -> '%s'\n", from, isdir == 0 ? to : tmp);
        }
        optind++;
        n_args--;
        from = argv[optind];
    } while (n_args > 0);

    return EXIT_SUCCESS;
}

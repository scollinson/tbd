//
//  src/recursive.c
//  tbd
//
//  Created by inoahdev on 12/02/18.
//  Copyright © 2018 - 2019 inoahdev. All rights reserved.
//

#include <errno.h>
#include <fcntl.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <unistd.h>

#include "path.h"
#include "recursive.h"

static
char *find_last_slash_before_end(char *const path, const char *const end) {
    const char *iter = path_find_last_row_of_slashes_before_end(path, end);
    return (char *)path_get_front_of_row_of_slashes(path, iter);
}

static char *find_next_slash_skipping_first_row(char *const path) {
    char *iter = path;
    char ch = *iter;

    do {
        ch = *(++iter);
    } while (ch == '/');

    for (char ch = *(++iter); ch != '\0'; ch = *(++iter)) {
        if (ch != '/') {
            continue;
        }
            
        /*
         * Skip past a row of slashes, if one does exist.
         */
            
        return (char *)path_get_back_of_row_of_slashes(iter);
    }

    return NULL;
}

static int
reverse_mkdir_ignoring_last(char *const path,
                            uint64_t path_length,
                            const mode_t mode,
                            char **const first_terminator_out)
{
    char *last_slash =
        (char *)path_find_last_row_of_slashes(path, path_length);

    /*
     * We may have a slash at the end of the string, which must be removed, so
     * we can properly iterate backwards the slashes that seperate components. 
     */

    int first_ret = 0;
    if (last_slash[1] == '\0') {
        last_slash =
            (char *)path_get_front_of_row_of_slashes(path, last_slash);
        
        *last_slash = '\0';
        first_ret = mkdir(path, mode);
        *last_slash = '/';

        last_slash = find_last_slash_before_end(path, last_slash);
    } else {
        *last_slash = '\0';
        first_ret = mkdir(path, mode);
        *last_slash = '/';
    }

    if (first_ret == 0) {
        return 0;
    }

    /*
     * If the directory already exists, return successfully.
     * 
     * Otherwise, if another error besides ENOENT (error given when a directory
     * in the hierarchy doesn't exist), return as fail.
     */

    if (first_ret < 0) {
        if (errno == EEXIST) {
            return 0;
        }

        if (errno != ENOENT) {
            return 1;
        }
    }

    /*
     * Store a pointer to the slash mentioned above, however, store at the back
     * of any row if one exists as we move backwards when iterating and
     * when checking against this variable.
     */

    char *const final_slash =
        (char *)path_get_back_of_row_of_slashes(last_slash);

    /*
     * Iterate over the path-components backwayds, finding the final slash
     * within a range of the of the previous final slash, and terminate the
     * string at that slash.
     * 
     * Then try creating that directory.
     * If that succeeds, break out of the loop to then iterate forwards and
     * create the directories afterwards.
     * 
     * If the directory already exists, we can simply return as it cannot happen
     * unless the second-to-last path-component already exists.
     * 
     * If a directory-component doesn't exist (ENOENT), continue with the
     * iteration.
     */

    while (last_slash != path) {
        last_slash = find_last_slash_before_end(path, last_slash);
        if (last_slash == NULL) {
            return 1;
        }

        *last_slash = '\0';
        
        const int ret = mkdir(path, mode);
        if (ret == 0) {
            *last_slash = '/';
            break;
        }
        
        if (ret < 0) {
            /*
             * If the directory already exists, we are done, as the previous
             * mkdir should have gone through.
             */

            if (errno == EEXIST) {
                *last_slash = '/';
                return 0;
            }

            /*
             * errno is set to ENONENT when a previous path-component doesn't
             * exist. So if we get any other error, its due to another reason,
             * and we just should return immedietly.
             */

            if (errno != ENOENT) {
                *last_slash = '/';
                return 1;
            }
        }

        *last_slash = '/';
    }

    if (first_terminator_out != NULL) {
        *first_terminator_out = last_slash;
    }

    /*
     * If last_slash is equal to final_slash, we have created the last
     * path-component we needed to, and should return.
     */

    if (last_slash == final_slash) {
        return 0;
    }

    /*
     * Get the next slash following the last-slash, to get the next
     * path-component after the last only that was just created.
     */

    char *slash = find_next_slash_skipping_first_row(last_slash);

    /*
     * Iterate forwards to create path-components following the final
     * path-component created in the previous loop.
     * 
     * Note: We still terminate at final_slash, and only afterwards do we stop,
     * as terminating at final_slash is needed to create the second-to-last 
     * path-component.
     */

    do {
        *slash = '\0';

        const int ret = mkdir(path, mode);
        if (ret < 0) {
            *slash = '/';
            return 1;
        }

        /*
         * Reset our slash, and if we've just created our final path-component,
         * break out and return.
         */

        *slash = '/';
        if (slash == final_slash) {
            break;
        }

        slash = find_next_slash_skipping_first_row(slash);
    } while (true);

    return 0;
}

int
open_r(char *const path,
       const uint64_t length,
       const mode_t flags,
       const mode_t mode,
       const mode_t dir_mode,
       char **const terminator_out)
{    
    int fd = open(path, O_CREAT | flags, mode);
    if (fd >= 0) {
        return fd;
    }

    if (errno != ENOENT) {
        return -1;
    }

    if (reverse_mkdir_ignoring_last(path, length, dir_mode, terminator_out)) {
        return -1;
    }

    fd = open(path, O_CREAT | flags, mode);
    if (fd < 0) {
        return -1;
    }

    return fd;
}

int
mkdir_r(char *const path,
        const uint64_t length,
        const mode_t mode,
        char **const first_terminator_out)
{
    if (mkdir(path, mode) == 0) {
        return 0;
    }
    
    if (errno == EEXIST) {
         return 0;
    }

    if (errno != ENOENT) {
        return 1;
    }

    if (reverse_mkdir_ignoring_last(path, length, mode, first_terminator_out)) {
        return 1;
    }

    if (mkdir(path, mode) < 0) {
        return 1;
    }

    return 0;
}

int
remove_partial_r(char *const path, const uint64_t length, char *const from) {
    if (remove(path) != 0) {
        return 1;
    }

    char *last_slash = (char *)path_find_last_row_of_slashes(from, length);

    do {
        if (last_slash == NULL) {
            return 1;
        }

        *last_slash = '\0';
        
        const int ret = remove(path);
        if (ret != 0) {
            return 1;
        }

        *last_slash = '/';
        last_slash = find_last_slash_before_end(from, last_slash);
    } while (true);

    return 0;
}

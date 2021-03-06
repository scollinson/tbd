//
//  src/tbd_for_main.h
//  tbd
//
//  Created by inoahdev on 11/30/18.
//  Copyright © 2018 - 2019 inoahdev. All rights reserved.
//

#ifndef TBD_FOR_MAIN_H
#define TBD_FOR_MAIN_H

#include <stdint.h>
#include "tbd.h"

enum tbd_for_main_dsc_image_flags {
    F_TBD_FOR_MAIN_DSC_IMAGE_FOUND_ONE = 1 << 0
};

struct tbd_for_main_dsc_image_filter {
    const char *string;

    uint64_t length;
    uint64_t flags;
};

struct tbd_for_main_dsc_image_path {
    const char *string;
    uint64_t flags;
};

enum tbd_for_main_options {
    O_TBD_FOR_MAIN_RECURSE_DIRECTORIES    = 1 << 0,
    O_TBD_FOR_MAIN_RECURSE_SUBDIRECTORIES = 1 << 1,

    O_TBD_FOR_MAIN_ADD_OR_REMOVE_ARCHS = 1 << 2,
    O_TBD_FOR_MAIN_ADD_OR_REMOVE_FLAGS = 1 << 3,

    O_TBD_FOR_MAIN_PRESERVE_DIRECTORY_SUBDIRS = 1 << 5,

    O_TBD_FOR_MAIN_NO_OVERWRITE           = 1 << 6,
    O_TBD_FOR_MAIN_REPLACE_PATH_EXTENSION = 1 << 7,

    O_TBD_FOR_MAIN_IGNORE_WARNINGS = 1 << 9,
    O_TBD_FOR_MAIN_NO_REQUESTS     = 1 << 10,

    O_TBD_FOR_MAIN_RECURSE_INCLUDE_DSC     = 1 << 11,
    O_TBD_FOR_MAIN_RECURSE_SKIP_IMAGE_DIRS = 1 << 12,

    /*
     * dyld_shared_cache extractions can be stored in either a file or a
     * directory. (Depending on the configuration)
     */

    O_TBD_FOR_MAIN_DSC_WRITE_PATH_IS_FILE = 1 << 13
};

enum tbd_for_main_filetype {
    TBD_FOR_MAIN_FILETYPE_MACHO,
    TBD_FOR_MAIN_FILETYPE_DYLD_SHARED_CACHE
};

struct tbd_for_main {
    struct tbd_create_info info;

    char *parse_path;
    char *write_path;

    /*
     * Store the paths' length here to avoid multiple strlen() calls on
     * the paths in main's dir_recurse callback.
     */

    uint64_t parse_path_length;
    uint64_t write_path_length;

    enum tbd_for_main_filetype filetype;

    /*
     * We store both option-sets for the filetypes as we need both when
     * recursing.
     */

    uint64_t macho_options;
    uint64_t dsc_options;

    uint64_t parse_options;
    uint64_t write_options;

    uint64_t options;

    /*
     * Archs and flags to either replace/remove ones found in a mach-o file.
     * Default is to replace.
     */

    uint64_t archs_re;
    uint64_t flags_re;

    struct array dsc_image_filters;
    struct array dsc_image_numbers;
    struct array dsc_image_paths;
};

bool
tbd_for_main_parse_option(struct tbd_for_main *tbd,
                          int argc,
                          const char *const *argv,
                          const char *option,
                          int *index);

/*
 * file_path_is_in_tbd asks whether file_path is from tbd->parse_path.
 */

char *
tbd_for_main_create_write_path(const struct tbd_for_main *tbd,
                               const char *folder_path,
                               uint64_t folder_path_length,
                               const char *file_path,
                               uint64_t file_path_length,
                               const char *extension,
                               uint64_t extension_length,
                               bool file_path_is_in_tbd,
                               uint64_t *length_out);
                               
void
tbd_for_main_apply_from(struct tbd_for_main *dst,
                        const struct tbd_for_main *src);

void
tbd_for_main_write_to_path(const struct tbd_for_main *tbd,
                           const char *input_path,
                           char *write_path,
                           uint64_t write_path_length,
                           bool print_paths);

void
tbd_for_main_write_to_stdout(const struct tbd_for_main *tbd,
                             const char *input_path,
                             bool print_paths);

void tbd_for_main_destroy(struct tbd_for_main *tbd);

#endif /* TBD_FOR_MAIN_H */

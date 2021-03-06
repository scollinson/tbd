//
//  src/guard_overflow.c
//  tbd
//
//  Created by inoahdev on 12/29/18.
//  Copyright © 2018 - 2019 inoahdev. All rights reserved.
//

#include "guard_overflow.h"

int
guard_overflow_shift_left_uint32(uint32_t *const left_in, const uint32_t right)
{
    const uint32_t left = *left_in;
    const uint32_t result = left << right;

    if (result < left) {
        return 1;
    }

    *left_in = result;
    return 0;
}

int
guard_overflow_shift_left_uint64(uint64_t *const left_in, const uint64_t right)
{
    const uint64_t left = *left_in;
    const uint64_t result = left << right;

    if (result < left) {
        return 1;
    }

    *left_in = result;
    return 0;
}

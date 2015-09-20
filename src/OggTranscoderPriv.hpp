/*
 *  OggTranscoderPriv.hpp
 *  OggTranscoder
 *
 *  Created by Saladfork on 9/19/15.
 *  Copyright © 2015 Saladfork. All rights reserved.
 *
 */

/* The classes below are not exported */
#pragma GCC visibility push(hidden)

#define OGGT_STREAM_SIZE        0x1000
#define OGGT_INITIAL_SIZE       0x10000
#define OGGT_BITS_PER_SECOND    16

#ifdef __APPLE__

#include <CoreFoundation/CFByteOrder.h>

#define _oggt_swap_int16(x) CFSwapInt16(x)
#define _oggt_swap_int32(x) CFSwapInt32(x)

#else

#define _oggt_swap_int16(x) (x)
#define _oggt_swap_int32(x) (x)

#endif

#pragma GCC visibility pop

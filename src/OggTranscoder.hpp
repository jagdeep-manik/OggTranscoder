/*
 *  OggTranscoder.hpp
 *  OggTranscoder
 *
 *  Created by Saladfork on 9/19/15.
 *  Copyright © 2015 Saladfork. All rights reserved.
 *
 */

#ifndef OggTranscoder_
#define OggTranscoder_

/* The classes below are exported */
#pragma GCC visibility push(default)

/*
 *  Contains the raw PCM data as well as
 *  its size in bytes.
 */
struct OggWav {
    unsigned long size = 0;
    void *data;
};

class OggTranscoder
{
	public:
        static OggWav transcode(void *ogg, unsigned long bytes);
};

#pragma GCC visibility pop
#endif

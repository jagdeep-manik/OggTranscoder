/*
 *  OggTranscoder.cpp
 *  OggTranscoder
 *
 *  Created by Saladfork on 9/19/15.
 *  Copyright © 2015 Saladfork. All rights reserved.
 *
 *  TODO:
 *      - Transcode metadata
 *      - Error handling
 *      - Support streaming data
 *
 *  DISCLAIMER:
 *      - Built for 16 bits per sample
 *
 */

#include <stdlib.h>
#include <string.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "OggTranscoder.hpp"
#include "OggTranscoderPriv.hpp"

/**
 *  WAVE File Header
 */
struct WavHeader {
    int chunkID;            // 'RIFF'
    int chunkSize;          // Total file size - 8 bytes
    int format;             // 'WAVE'
    int fmtID;              // 'fmt\0'
    int fmtChunkSize;       // Size of fmt chunk
    short audioFormat;      // PCM
    short numChannels;      // 1: Mono, 2: Stereo
    int sampleRate;         // Hz (ex: 44100)
    int byteRate;           // rate * bps * channels / 8
    short blockAlign;       // bps * channels / 8
    short bitsPerSample;    // Bits per sample (16)
    int dataID;             // 'data'
    int dataChunkSize;      // Size of data chunk
};

WavHeader constructHeader(int pcmLength, int sampleRate, short channels, short bps) {
    WavHeader header;
    header.chunkID       = _oggt_swap_int32(0x52494646);
    header.chunkSize     = pcmLength + 36;
    
    header.format        = _oggt_swap_int32(0x57415645);
    header.fmtID         = _oggt_swap_int32(0x666D7420);
    header.fmtChunkSize  = 16;
    
    header.audioFormat   = 1;
    header.numChannels   = channels;
    header.sampleRate    = sampleRate;
    header.byteRate      = (sampleRate * channels * bps) / 8;
    
    header.blockAlign    = (bps * channels) / 8;
    header.bitsPerSample = bps;
    
    header.dataID        = _oggt_swap_int32(0x64617461);
    header.dataChunkSize = pcmLength;
    return header;
}

/**
 *  Transcodes an ogg buffer to an uncompressed WAV-PCM.
 *
 *  - parameter ogg:      Pointer to ogg buffer
 *  - parameter bytes:    Size of buffer (in bytes)
 *
 *  - returns:  Uncompressed WAV-PCM buffer
 */
OggWav OggTranscoder::transcode(void *ogg, unsigned long bytes)
{
    OggWav result;
    if (bytes <= 0) {
        return result;
    }
    
    OggVorbis_File vf;
    char pcmout[OGGT_STREAM_SIZE];
    int eof = 0;
    int currentSection;
    
    /*
     *  Ogg-Vorbis accepts input as a file or pipe, but
     *  not as a buffer. Create a temporary file, copy
     *  the ogg into it, and pass it on.
     */
    FILE *oggFile = tmpfile();
    fwrite(ogg, 1, bytes, oggFile);
    rewind(oggFile);
    
    if (ov_open_callbacks(oggFile, &vf, NULL, 0, OV_CALLBACKS_NOCLOSE) < 0) {
        fprintf(stderr, "Input does not appear to be an Ogg bitstream.\n");
        return result;
    }
    
    vorbis_info *info = ov_info(&vf, -1);
    short numberOfChannels = (short) info->channels;
    short sampleRate = (int) info->rate;
    
    int accumSize = sizeof(WavHeader);
    int pcmSize = OGGT_INITIAL_SIZE;
    void *pcm = malloc(pcmSize);
    if (!pcm) {
        fprintf(stderr, "Out of memory.\n");
        return result;
    }
    
    /*
     *  Loop through the file, one vorbis packet at a time.
     *  If there is not enough room in the buffer, double its
     *  size and reallocate memory.
     */
    while (!eof) {
        long bytesRead = ov_read(&vf, pcmout, OGGT_STREAM_SIZE, 0, 2, 1, &currentSection);
        
        if (bytesRead == 0) {
            eof = 1;
        } else if (bytesRead < 0) {
            /* Miscellaneous error handling */
        } else {
            int currentSize = accumSize;
            accumSize += bytesRead;
            
            if (accumSize > pcmSize) {
                pcmSize *= 2;
                pcm = realloc(pcm, pcmSize);
                if (!pcm) {
                    fprintf(stderr, "Out of memory.\n");
                    return result;
                }
            }
            memcpy(((char*) pcm) + currentSize, pcmout, bytesRead);
        }
    }
    fclose(oggFile);
    
    /*
     *  Stitch the header and the raw PCM together.
     */
    WavHeader header = constructHeader(accumSize - sizeof(WavHeader), sampleRate, numberOfChannels, OGGT_BITS_PER_SECOND);
    memcpy(pcm, &header, sizeof(WavHeader));
    
    result.size = accumSize;
    result.data = pcm;
    return result;
};



/*
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Based on libavutil/base64.c
 */

#include <stdio.h>
#include <stdint.h>

// Base64 alphabet (standard variant)
static const char B64[64] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Encodes exactly 3 bytes into 4 Base64 characters
static void enc3(const unsigned char *in, char *out) {
    // Merge the 3 bytes into a single 24-bit integer
    uint32_t v = ((uint32_t)in[0] << 16) | ((uint32_t)in[1] << 8) | in[2];

    // Extract each 6-bit group and map it to a Base64 character
    out[0] = B64[(v >> 18) & 0x3F];
    out[1] = B64[(v >> 12) & 0x3F];
    out[2] = B64[(v >>  6) & 0x3F];
    out[3] = B64[(v >>  0) & 0x3F];
}

int main(void) {
    // Input buffer size: multiple of 3 to simplify encoding
    // Here: 64 KiB minus a few bytes to make it divisible by 3
    enum { IN_CHUNK = 65520 }; // 65520 % 3 == 0

    unsigned char inbuf[IN_CHUNK];
    // Output buffer: each 3 bytes → 4 Base64 chars, plus some extra space
    char outbuf[(IN_CHUNK / 3) * 4 + 8];

    size_t n;
    while ((n = fread(inbuf, 1, IN_CHUNK, stdin)) > 0) {
        size_t full = n / 3 * 3; // Number of bytes that form complete 3-byte groups
        size_t oi = 0;           // Output buffer index

        // Encode all full 3-byte groups
        for (size_t i = 0; i < full; i += 3) {
            enc3(&inbuf[i], &outbuf[oi]);
            oi += 4;
        }

        // Handle the remainder (0, 1, or 2 leftover bytes)
        size_t rem = n - full;
        if (rem) {
            unsigned char a = inbuf[full + 0];
            unsigned char b = (rem == 2) ? inbuf[full + 1] : 0;
            uint32_t v = ((uint32_t)a << 16) | ((uint32_t)b << 8);

            outbuf[oi + 0] = B64[(v >> 18) & 0x3F];
            outbuf[oi + 1] = B64[(v >> 12) & 0x3F];
            outbuf[oi + 2] = (rem == 2) ? B64[(v >> 6) & 0x3F] : '=';
            outbuf[oi + 3] = '=';
            oi += 4;
        }

        // Write the encoded chunk to stdout
        fwrite(outbuf, 1, oi, stdout);
    }

    // Final newline
    fputc('\n', stdout);
    return 0;
}


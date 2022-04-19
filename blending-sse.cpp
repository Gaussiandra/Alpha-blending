#include "blending-sse.hpp"
#include <cassert>

inline unsigned char *pixelPtr(unsigned char pic[], unsigned x, 
                             unsigned y, unsigned width) {
    return pic + (y * width + x) * 4;
}

void blendPics(unsigned char back[], unsigned char front[], unsigned char blendedPic[], 
               unsigned frontHeight, unsigned frontWidth, unsigned backWidth,
               unsigned backX, unsigned backY) {
    assert(back);
    assert(front);
    assert(blendedPic);

    for (int y = 0; y < frontHeight; y++) { 
        for (int x = 0; x < frontWidth; x += 4) {
            //-----------------------------------------------------------------------
            //       15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
            // fr = [r3 g3 b3 a3 | r2 g2 b2 a2 | r1 g1 b1 a1 | r0 g0 b0 a0]
            //-----------------------------------------------------------------------

            __m128i fr = _mm_load_si128((__m128i*) pixelPtr(front, x, y, frontWidth));
            __m128i bk = _mm_load_si128((__m128i*) pixelPtr(back, x + backX, y + backY, backWidth));                  

            // Move the upper 2 single-precision (32-bit) floating-point elements 
            // from b to the lower 2 elements of dst, 
            // and copy the upper 2 elements from a to the upper 2 elements of dst.
            //-----------------------------------------------------------------------
            //       15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
            // fr = [a3 r3 g3 b3 | a2 r2 g2 b2 | a1 r1 g1 b1 | a0 r0 g0 b0]
            //        \  \  \  \    \  \  \  \   xx xx xx xx   xx xx xx xx            
            //         \  \  \  \    \  \  \  \.
            //          \  \  \  \    '--+--+--+-------------+--+--+--.
            //           '--+--+--+------------+--+--+--.     \  \  \  \.
            //                                  \  \  \  \     \  \  \  \.
            // FR = [-- -- -- -- | -- -- -- -- | a3 r3 g3 b3 | a2 r2 g2 b2]
            //-----------------------------------------------------------------------
            
            __m128i FR = (__m128i) _mm_movehl_ps((__m128) _0, (__m128) fr);          // FR = (fr >> 8*8)
            __m128i BK = (__m128i) _mm_movehl_ps((__m128) _0, (__m128) bk);       

            //-----------------------------------------------------------------------
            //       15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
            // fr = [a3 r3 g3 b3 | a2 r2 g2 b2 | a1 r1 g1 b1 | a0 r0 g0 b0]
            //       xx xx xx xx   xx xx xx xx                 /  /   |  |
            //                                         _______/  /   /   |
            //            ...   ...     ...           /     ____/   /    |
            //           /     /       /             /     /       /     |
            // fr = [-- a1 -- r1 | -- g1 -- b1 | -- a0 -- r0 | -- g0 -- b0]
            //-----------------------------------------------------------------------

            fr = _mm_cvtepu8_epi16(fr);                                              // fr[i] = (WORD) fr[i]
            FR = _mm_cvtepu8_epi16(FR);                                           

            bk = _mm_cvtepu8_epi16(bk);
            BK = _mm_cvtepu8_epi16(BK);

            //-----------------------------------------------------------------------
            //       15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
            // fr = [-- a1 -- r1 | -- g1 -- b1 | -- a0 -- r0 | -- g0 -- b0]
            //          |___________________        |___________________
            //          |     \      \      \       |     \      \      \.
            // a  = [-- a1 -- a1 | -- a1 -- a1 | -- a0 -- a0 | -- a0 -- a0]
            //-----------------------------------------------------------------------

            static const __m128i moveA = _mm_set_epi8(Z, 0xEu, Z, 0xEu, Z, 0xEu, Z, 0xEu, 
                                                      Z, 0x6u, Z, 0x6u, Z, 0x6u, Z, 0x6u);
            __m128i a = _mm_shuffle_epi8(fr, moveA);                                
            __m128i A = _mm_shuffle_epi8(FR, moveA);
            
            //-----------------------------------------------------------------------

            fr = _mm_mullo_epi16(fr, a);                                             // fr *= a mullo
            FR = _mm_mullo_epi16(FR, A);

            bk = _mm_mullo_epi16(bk, _mm_sub_epi8(_255, a));                         // bk *= (255-a)  sub
            BK = _mm_mullo_epi16(BK, _mm_sub_epi8(_255, A));

            __m128i sum = _mm_add_epi16(fr, bk);                                     // sum = fr*a + bk*(255-a)
            __m128i SUM = _mm_add_epi16(FR, BK);

            //-----------------------------------------------------------------------
            //        15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
            // sum = [A1 a1 R1 r1 | G1 g1 B1 b1 | A0 a0 R0 r0 | G0 g0 B0 b0]
            //         \     \       \     \       \_____\_______\_____\.
            //          \_____\_______\_____\______________    \  \  \  \.
            //                                    \  \  \  \    \  \  \  \.
            // sum = [-- -- -- -- | -- -- -- -- | A1 R1 G1 B1 | A0 R0 G0 B0]
            //-----------------------------------------------------------------------

            static const __m128i moveSum = _mm_set_epi8(Z,    Z,    Z,    Z,    Z,    Z,    Z,    Z, 
                                                        0xFu, 0xDu, 0xBu, 0x9u, 0x7u, 0x5u, 0x3u, 0x1u);
            sum = _mm_shuffle_epi8(sum, moveSum);                                      
            SUM = _mm_shuffle_epi8(SUM, moveSum);                                    // sum[i] = (sium[i] >> 8) = (sum[i] / 256)
            
            // Move the lower 2 single-precision (32-bit) 
            // floating-point elements from b to the upper 2 elements of dst, 
            // and copy the lower 2 elements from a to the lower 2 elements of dst.
            //-----------------------------------------------------------------------
            //          15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
            // sum   = [-- -- -- -- | -- -- -- -- | a1 r1 g1 b1 | a0 r0 g0 b0] ->-.
            // sumHi = [-- -- -- -- | -- -- -- -- | a3 r3 g3 b3 | a2 r2 g2 b2]    |
            //                                      /  /  /  /    /  /  /  /      V
            //             .--+--+--+----+--+--+--++--+--+--+----+--+--+--'       |
            //            /  /  /  /    /  /  /  /    ____________________________/
            //           /  /  /  /    /  /  /  /    /  /  /  /    /  /  /  /
            // color = [a3 r3 g3 b3 | a2 r2 g2 b2 | a1 r1 g1 b1 | a0 r0 g0 b0]
            //-----------------------------------------------------------------------

            __m128i color = (__m128i) _mm_movelh_ps((__m128) sum, (__m128) SUM);     // color = (sumHi << 8*8) | sum

            static const __m128i ones = _mm_set_epi8(1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u);
            color = _mm_add_epi8(color, ones);

            _mm_store_si128 ((__m128i*) pixelPtr(blendedPic, backX + x, backY + y, backWidth), color);
        }
    }
}
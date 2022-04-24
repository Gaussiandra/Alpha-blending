#include <immintrin.h>

const char I = 255u,
           Z = 0x80u;
           
const __m128i   _0 =                    _mm_set_epi8 (0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0);
const __m128i _255 = _mm_cvtepu8_epi16 (_mm_set_epi8 (I,I,I,I, I,I,I,I, I,I,I,I, I,I,I,I));

void blendPics(unsigned char back[], unsigned char front[], unsigned char blendedPic[], 
               unsigned frontHeight, unsigned frontWidth, unsigned backWidth,
               unsigned backX, unsigned backY);

inline unsigned char *pixelPtr(unsigned char pic[], unsigned x, 
                               unsigned y, unsigned width);
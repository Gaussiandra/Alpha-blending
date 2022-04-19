void blendPics(unsigned char back[], unsigned char front[], unsigned char blendedPic[], 
               unsigned frontHeight, unsigned frontWidth, unsigned backWidth,
               unsigned backX, unsigned backY) {
    
    for (int i = 0; i < frontHeight; ++i) {
        for (int j = 0; j < frontWidth; ++j) {
            unsigned back_i = backY + i, back_j = backX + j;
            unsigned char frontTransparency = front[(i * frontWidth + j) * 4 + 3];

            for (int color = 0; color < 3; ++color) {
                unsigned char frontColor = front[(i * frontWidth + j) * 4 + color];
                unsigned char backColor  =  back[(back_i * backWidth + back_j) * 4 + color];

                blendedPic[(back_i * backWidth + back_j) * 4 + color] = 
                    (backColor * (255 - frontTransparency) + frontColor * frontTransparency) / 255;
            
            }
        }
    }
}
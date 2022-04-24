#include <SFML/Graphics.hpp>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>

#if (SSE)
    #include "blending-sse.hpp"
#else
    #include "blending-nosse.hpp"
#endif

const unsigned int FPS_STRING_LEN = 50;
  
int main()
{
    sf::Image back, front;
    assert(back.loadFromFile("Table.bmp"));
    assert(front.loadFromFile("Racket.bmp"));

    unsigned char *backAlignedCopy = (unsigned char*) 
        aligned_alloc(16, back.getSize().x * back.getSize().y * 4);
    memcpy(backAlignedCopy, back.getPixelsPtr(), back.getSize().x * back.getSize().y * 4);

    unsigned char *frontAlignedCopy = (unsigned char*) 
        aligned_alloc(16, front.getSize().x * front.getSize().y * 4);
    memcpy(frontAlignedCopy, front.getPixelsPtr(), front.getSize().x * front.getSize().y * 4);

    assert(back.getSize().x >= front.getSize().x);
    assert(back.getSize().y >= front.getSize().y);
    unsigned char* blendedPic = (unsigned char*) aligned_alloc(16,
        back.getSize().x * back.getSize().y * 4 * sizeof(unsigned char));
    memcpy(blendedPic, back.getPixelsPtr(), back.getSize().x * back.getSize().y * 4);

    sf::RenderWindow window(sf::VideoMode(back.getSize().x, back.getSize().y), "Alpha blending");

    sf::Font font;
    assert(font.loadFromFile("Lato-Black.ttf"));
    sf::Text fpsText;
    fpsText.setFont(font);
    fpsText.setCharacterSize(20);
    fpsText.setFillColor(sf::Color::White);

    char fpsString[FPS_STRING_LEN] = {};
    float curFps = 0, smoothedFps = 0;
    sf::Clock clock = sf::Clock();
    sf::Time previousTime = clock.getElapsedTime();
    sf::Time currentTime = sf::Time::Zero;

    sf::Texture texture;
    texture.create(back.getSize().x, back.getSize().y);
    sf::Sprite sprite(texture);

    while (window.isOpen())
    { 
        sf::Event event;
        while (window.pollEvent(event))
        {   
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
            window.close();
        }   

        blendPics(backAlignedCopy, 
                frontAlignedCopy, blendedPic, 
                front.getSize().y, front.getSize().x, back.getSize().x,
                420, 232);
        texture.update(blendedPic);

        window.clear();
        window.draw(sprite);

        currentTime = clock.getElapsedTime();
        curFps = 1.0f / (currentTime.asSeconds() - previousTime.asSeconds());
        smoothedFps = smoothedFps * 0.99 + curFps * 0.01;
        sprintf(fpsString, "fps = %.1f", smoothedFps);
        fpsText.setString(fpsString);
        previousTime = currentTime;
        window.draw(fpsText);

        window.display();
    }
    // sse O0 2080, nosse O0 1250, x1.6
    // sse O2 2020 nosse O2 1270

    free(backAlignedCopy);
    free(frontAlignedCopy);

    return 0;
}

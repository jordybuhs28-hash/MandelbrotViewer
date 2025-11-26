// mandelbrot.cpp
// Build: g++ -std=c++17 mandelbrot.cpp -o mandelbrot -lsfml-graphics -lsfml-window -lsfml-system
#include <SFML/Graphics.hpp>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

struct ViewState {
    double centerX = -0.5;
    double centerY = 0.0;
    double zoom    = 300.0;  // pixels per unit
    int    maxIter = 200;
};

static sf::Color palette(double t) {
    // Smooth gradient palette
    double r = 9*(1-t)*t*t*t*255;
    double g = 15*(1-t)*(1-t)*t*t*255;
    double b = 8.5*(1-t)*(1-t)*(1-t)*t*255;
    return sf::Color(
        static_cast<sf::Uint8>(std::clamp(r, 0.0, 255.0)),
        static_cast<sf::Uint8>(std::clamp(g, 0.0, 255.0)),
        static_cast<sf::Uint8>(std::clamp(b, 0.0, 255.0))
    );
}

static double smoothEscape(double zx, double zy, int iter, int maxIter) {
    if (iter >= maxIter) return 0.0;
    double mag = std::sqrt(zx*zx + zy*zy);
    double nu = std::log2(std::log(mag + 1e-16));
    double val = (iter + 1 - nu) / maxIter;
    return std::clamp(val, 0.0, 1.0);
}

int main() {
    const int W = 800, H = 600;
    sf::RenderWindow window(sf::VideoMode(W, H), "Mandelbrot Viewer");
    window.setFramerateLimit(60);

    ViewState view;
    sf::Image image; image.create(W, H, sf::Color::Black);
    sf::Texture texture; sf::Sprite sprite;

    bool needsRender = true;
    bool dragging = false;
    sf::Vector2i dragStart;
    double dragStartX, dragStartY;

    sf::Font font; bool fontOk = font.loadFromFile("SodaHendrix.ttf");

    auto render = [&]() {
        for (int py = 0; py < H; ++py) {
            for (int px = 0; px < W; ++px) {
                double x0 = view.centerX + (px - W/2) / view.zoom;
                double y0 = view.centerY + (py - H/2) / view.zoom;

                double zx = 0.0, zy = 0.0;
                int iter = 0;
                while (iter < view.maxIter) {
                    double zx2 = zx*zx - zy*zy + x0;
                    double zy2 = 2*zx*zy + y0;
                    zx = zx2; zy = zy2;
                    if (zx*zx + zy*zy > 4.0) break;
                    ++iter;
                }
                if (iter == view.maxIter) {
                    image.setPixel(px, py, sf::Color::Black);
                } else {
                    double t = smoothEscape(zx, zy, iter, view.maxIter);
                    image.setPixel(px, py, palette(t));
                }
            }
        }
        texture.loadFromImage(image);
        sprite.setTexture(texture, true);
        needsRender = false;
        std::cout << "Render complete: Center(" << view.centerX << "," << view.centerY
                  << ") Zoom=" << view.zoom << " Iter=" << view.maxIter << "\n";
    };

    auto hudText = [&](sf::Font &font) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6)
            << "Center: (" << view.centerX << ", " << view.centerY << ")  "
            << "Zoom: " << view.zoom << " px/unit  "
            << "MaxIter: " << view.maxIter << "\n"
            << "Controls: Wheel/+/ - zoom | Drag/Arrows pan | R reset | [ ] iterations";
        sf::Text text(oss.str(), font, 14);
        text.setFillColor(sf::Color::White);
        text.setOutlineColor(sf::Color(0,0,0,160));
        text.setOutlineThickness(2.0f);
        text.setPosition(10.f, 10.f);
        return text;
    };

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            else if (event.type == sf::Event::MouseWheelScrolled) {
                sf::Vector2i mpos = sf::Mouse::getPosition(window);
                double mx = view.centerX + (mpos.x - W/2) / view.zoom;
                double my = view.centerY + (mpos.y - H/2) / view.zoom;
                double factor = (event.mouseWheelScroll.delta > 0) ? 1.25 : 0.8;
                view.zoom *= factor;
                double mxNew = view.centerX + (mpos.x - W/2) / view.zoom;
                double myNew = view.centerY + (mpos.y - H/2) / view.zoom;
                view.centerX += (mx - mxNew);
                view.centerY += (my - myNew);
                needsRender = true;
            }
            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                dragging = true;
                dragStart = sf::Mouse::getPosition(window);
                dragStartX = view.centerX; dragStartY = view.centerY;
            }
            else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                dragging = false;
            }
            else if (event.type == sf::Event::MouseMoved && dragging) {
                sf::Vector2i cur = sf::Mouse::getPosition(window);
                sf::Vector2i delta = cur - dragStart;
                view.centerX = dragStartX - delta.x / view.zoom;
                view.centerY = dragStartY - delta.y / view.zoom;
                needsRender = true;
            }
            else if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                    case sf::Keyboard::Add: case sf::Keyboard::Equal: view.zoom *= 1.25; needsRender = true; break;
                    case sf::Keyboard::Dash:
                    case sf::Keyboard::Subtract:
                        view.zoom *= 0.8; needsRender = true; break;
                    case sf::Keyboard::Left: view.centerX -= 50.0 / view.zoom; needsRender = true; break;
                    case sf::Keyboard::Right: view.centerX += 50.0 / view.zoom; needsRender = true; break;
                    case sf::Keyboard::Up: view.centerY -= 50.0 / view.zoom; needsRender = true; break;
                    case sf::Keyboard::Down: view.centerY += 50.0 / view.zoom; needsRender = true; break;
                    case sf::Keyboard::R: view = ViewState{}; needsRender = true; break;
                    case sf::Keyboard::LBracket: view.maxIter = std::max(50, view.maxIter - 50); needsRender = true; break;
                    case sf::Keyboard::RBracket: view.maxIter = std::min(5000, view.maxIter + 50); needsRender = true; break;
                    default: break;
                }
            }
        }
        if (needsRender) render();
        window.clear();
        window.draw(sprite);
        if (fontOk) window.draw(hudText(font));
        window.display();
    }
    return 0;
}

#include <iostream>
#include <SFML/Graphics.hpp>

using namespace std;

int main(int argc, char* argv[])
{
  sf::RenderWindow window(sf::VideoMode(200, 200), "My Window");
  sf::CircleShape shape(100.0f);
  shape.setFillColor(sf::Color::Green);

  // Main window loop, while window is open
  while(window.isOpen()) {
    sf::Event event;

    while(window.pollEvent(event)) {
      if(event.type == sf::Event::Closed) {
	window.close();
      }
    }

    window.clear();
    window.draw(shape);
    window.display();
  }
  
  return 0;
}

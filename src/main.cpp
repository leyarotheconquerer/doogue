#include <iostream>
#include <sstream>

#include <SFML/Graphics.hpp>

using namespace std;

int main(int argc, char* argv[])
{
  stringstream buffer;

  sf::Clock typingTimer;
  
  sf::RenderWindow window(sf::VideoMode(200, 200), "My Window", sf::Style::None);
  sf::CircleShape shape(100.0f);
  shape.setFillColor(sf::Color::Green);

  // Main window loop, while window is open
  while(window.isOpen()) {
    sf::Event event;

    while(window.pollEvent(event)) {
      switch(event.type) {
      case sf::Event::Closed:
	window.close();
	break;

      case sf::Event::LostFocus:
	cout << "Lost focus!" << endl;
	break;

      case sf::Event::GainedFocus:
	cout << "Gained focus!" << endl;
	break;

      case sf::Event::TextEntered:
	buffer << static_cast<char>(event.text.unicode);
	typingTimer.restart();
	
	break;
      }
    }

    if(buffer.str().length() > 0 && typingTimer.getElapsedTime().asSeconds() >= 0.5f) {
      cout << "Typed: " << buffer.str() << endl;
      buffer.str("");
    }

    window.clear();
    window.draw(shape);
    window.display();
  }
  
  return 0;
}

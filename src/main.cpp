#include <iostream>
#include <sstream>

#include <SFML/Graphics.hpp>

using namespace std;

int main(int argc, char* argv[])
{
  bool paused = false;

  sf::Clock gameClock;
  
  sf::RenderWindow window(sf::VideoMode(200, 200), "My Window", sf::Style::None);

  // Player Setup
  sf::CircleShape player_shape(4.0f);
  player_shape.setFillColor(sf::Color::Red);

  // Main window loop, while window is open
  while(window.isOpen()) {
    sf::Event event;

    while(window.pollEvent(event)) {
      switch(event.type) {
      case sf::Event::Closed:
	window.close();
	break;

      case sf::Event::LostFocus:
	paused = true;
	cout << "Paused!" << endl;
	break;

      case sf::Event::GainedFocus:
	paused = false;
	cout << "Resumed..." << endl;
	break;
    }
      
    // Game time!!
    sf::Time deltaTime = gameClock.restart();

    if(!paused) {
      // Game logic
    }

    // Render game
    window.clear();
    window.draw(shape);
    window.display();
  }
  
  return 0;
}

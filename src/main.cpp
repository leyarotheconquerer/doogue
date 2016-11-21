#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <sstream>

#include <SFML/Graphics.hpp>

#define PLAYER_ACCEL 42.0f
#define PLAYER_FRICTION 28.0f
#define PLAYER_MAXSPEED 8.0f
#define PLAYER_LOOK_SENSITIVITY 1.0f
#define PLAYER_TURNRATE 4.5f

using namespace std;

float magnitude(sf::Vector2f vector);
float sqrMagnitude(sf::Vector2f vector);

int main(int argc, char* argv[])
{
  bool paused = false;
  bool mouse_locked = true;

  sf::Clock gameClock;
  
  sf::RenderWindow window(sf::VideoMode(200, 200), "My Window", sf::Style::None);

  // Player Setup
  float player_rotation = 0.0f;
  sf::Vector2f player_speed(0.0f, 0.0f);
  sf::Vector2f player_position(0.0f, 0.0f);
  sf::CircleShape player_shape(4.0f);
  player_shape.setFillColor(sf::Color::Red);

  // Mouse Position
  sf::Vector2f mouse_previous;

  // Main window loop, while window is open
  while(window.isOpen()) {
    sf::Event event;

    while(window.pollEvent(event)) {
      switch(event.type) {
      case sf::Event::KeyReleased:
	if(event.key.code == sf::Keyboard::L) {
	  mouse_locked = !mouse_locked;
	}
	
	break;
	
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
    }
      
    // Game time!!
    sf::Time deltaTime = gameClock.restart();

    if(!paused) {
      // Game logic

      // Player input
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
	player_speed.y += PLAYER_ACCEL*deltaTime.asSeconds();
      }

      if(sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
	player_speed.y -= PLAYER_ACCEL*deltaTime.asSeconds();
      }

      if(sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
	player_speed.x += PLAYER_ACCEL*deltaTime.asSeconds();
      }

      if(sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
	player_speed.x -= PLAYER_ACCEL*deltaTime.asSeconds();
      }

      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
	player_rotation -= PLAYER_TURNRATE*deltaTime.asSeconds();
      }

      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
	player_rotation += PLAYER_TURNRATE*deltaTime.asSeconds();
      }

      // Mouse movement
      sf::Vector2f mouse_current((float)sf::Mouse::getPosition().x, (float)sf::Mouse::getPosition().y);
      sf::Vector2f mouse_delta = mouse_current - mouse_previous;
      mouse_previous = mouse_current;

      if(mouse_locked) {
	sf::Mouse::setPosition(sf::Vector2i(100, 100), window);
      }

      player_rotation += PLAYER_LOOK_SENSITIVITY*mouse_delta.x*deltaTime.asSeconds();

      cout << "Rotation: " << player_rotation << endl;

      // Player physics
      if(sqrMagnitude(player_speed) > 0.0f) {
	float speed_magnitude = magnitude(player_speed);

	// Apply friction
	if(speed_magnitude <= PLAYER_FRICTION*deltaTime.asSeconds()) {
	  player_speed.x = 0.0f;
	  player_speed.y = 0.0f;
	} else {
	  player_speed -= (player_speed/speed_magnitude)*PLAYER_FRICTION*deltaTime.asSeconds();
	}

	// Cap player speed
	// TODO: Replace with proper mathematical approach code
	if(speed_magnitude > PLAYER_MAXSPEED) {
	  player_speed = PLAYER_MAXSPEED*(player_speed/speed_magnitude);
	}
	
	// Apply player speed
	player_position += player_speed*deltaTime.asSeconds();
	player_shape.setPosition(player_position);
      }

      // Player rotation
      player_shape.setRotation(player_rotation);
    }

    // Render game
    window.clear();
    window.draw(player_shape);
    window.display();
  }
  
  return 0;
}

float magnitude(sf::Vector2f vector)
{
  return sqrt(vector.x*vector.x + vector.y*vector.y);
}

float sqrMagnitude(sf::Vector2f vector)
{
  return vector.x*vector.x + vector.y*vector.y;
}

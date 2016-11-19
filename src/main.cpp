#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <vector>
#include <sstream>

#include <SFML/Graphics.hpp>

#define PI 3.1415926535f
#define DEG_TO_RAD PI/180.0f

#define PLAYER_ACCEL 42.0f
#define PLAYER_FRICTION 28.0f
#define PLAYER_MAXSPEED 8.0f

#define WINDOW_RESOLUTION_WIDTH 1024
#define WINDOW_RESOLUTION_HEIGHT 768

#define RAYCAST_FOV 60
#define RAYCAST_VIEWDISTANCE 25.0f

using namespace std;

struct line {
  sf::Vector2f first;
  sf::Vector2f second;

  line(sf::Vector2f first_init, sf::Vector2f second_init):
    first(first_init), second(second_init) {}
};

float magnitude(sf::Vector2f vector);
float sqrMagnitude(sf::Vector2f vector);

int main(int argc, char* argv[])
{
  bool paused = false;

  sf::Clock gameClock;
  
  sf::RenderWindow window(sf::VideoMode(WINDOW_RESOLUTION_WIDTH, WINDOW_RESOLUTION_HEIGHT), "My Window", sf::Style::None);

  // Map Setup
  vector<line> map;
  
  map.push_back(line(sf::Vector2f(-5.0f, -5.0f), sf::Vector2f(-5.0f, 5.0f)));
  map.push_back(line(sf::Vector2f(-5.0f, 5.0f), sf::Vector2f(5.0f, 5.0f)));
  map.push_back(line(sf::Vector2f(5.0f, 5.0f), sf::Vector2f(5.0f, -5.0f)));
  map.push_back(line(sf::Vector2f(5.0f, -5.0f), sf::Vector2f(-5.0f, -5.0f)));

  // Player Setup
  sf::Vector2f player_speed(0.0f, 0.0f);
  sf::Vector2f player_position(0.0f, 0.0f);
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
    }

    // Render game
    window.clear();
    window.draw(player_shape);

    // Raycast walls
    float slice_width = (float)RAYCAST_FOV/WINDOW_RESOLUTION_WIDTH;
    
    for(int slice = 0; slice < WINDOW_RESOLUTION_WIDTH; ++slice) {
      float slice_angle = slice_width*slice;
      float distance = -1.0f;

      for(auto& wall : map) {
	sf::Vertex line[] = { sf::Vertex(scale*(wall.first + offset)), sf::Vertex(scale*(wall.second + offset))  };
	
	window.draw(line, 2, sf::Lines);
      }

      // Draw wall
      if(distance >= 0.0f) {
	float height = distance
	window.draw(
      }
    }
    
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

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
#define RAYCAST_WALLHEIGHT 700.0f

using namespace std;

struct line {
  sf::Vector2f first;
  sf::Vector2f second;

  line(sf::Vector2f first_init, sf::Vector2f second_init):
    first(first_init), second(second_init) {}
};

float magnitude(sf::Vector2f vector);
float sqrMagnitude(sf::Vector2f vector);
bool findIntersection(const line a, const line b, sf::Vector2f* intersection);
bool findIntersection(const sf::Vector2f a_first, const sf::Vector2f a_second, const sf::Vector2f b_first, const sf::Vector2f b_second, sf::Vector2f* intersection);

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
      }
    }

    // Render game
    window.clear();

    // Raycast walls
    int slice_count = WINDOW_RESOLUTION_WIDTH;
    float slice_width = (float)RAYCAST_FOV/WINDOW_RESOLUTION_WIDTH;
    vector<sf::Vertex> slice_points(2*WINDOW_RESOLUTION_WIDTH);

    //*
    for(int slice = 0; slice < WINDOW_RESOLUTION_WIDTH; ++slice) {
      float slice_angle = slice_width*slice;
      float sqr_distance = -1.0f;

      sf::Vector2f intersection;
      sf::Vector2f slice_target(RAYCAST_VIEWDISTANCE*sin(DEG_TO_RAD*slice_angle), RAYCAST_VIEWDISTANCE*cos(DEG_TO_RAD*slice_angle));
      line slice_segment(player_position, player_position + slice_target);
      
      for(auto& wall : map) {
	if(findIntersection(slice_segment, wall, &intersection)) {
	  float intersection_distance = sqrMagnitude(intersection - player_position);
	  
	  if(sqr_distance < 0.0f || intersection_distance < sqr_distance) {
	    sqr_distance = intersection_distance;
	  }
	}
      }

      // Draw wall
      if(sqr_distance >= 0.0f) {
	float distance = sqrt(sqr_distance);
	float height = WINDOW_RESOLUTION_HEIGHT/(distance < 0.05f ? 0.05f : distance);

	float start = -height/2+WINDOW_RESOLUTION_HEIGHT/2;
	float end = height/2+WINDOW_RESOLUTION_HEIGHT/2;

	slice_points[2*slice] = sf::Vertex(sf::Vector2f(slice, start));
	slice_points[2*slice + 1] = sf::Vertex(sf::Vector2f(slice, end));
      }
    }
    //*/

    window.draw(&slice_points[0], 2*WINDOW_RESOLUTION_WIDTH, sf::Lines);

    // Draw a minimap for the sake of debugging
    float minimap_scale = 10.0f;
    sf::Vector2f minimap_offset(WINDOW_RESOLUTION_WIDTH/2, WINDOW_RESOLUTION_HEIGHT/2);

    // -- Draw player
    player_shape.setPosition(minimap_offset + minimap_scale*player_position - (player_shape.getRadius() + player_shape.getOutlineThickness())*sf::Vector2f(1.0f, 1.0f));
    window.draw(player_shape);

    // -- Draw walls
    for(auto& wall : map) {
      sf::Vertex line[] = { minimap_offset + minimap_scale*wall.first, minimap_offset + minimap_scale*wall.second };
      window.draw(line, 2, sf::Lines);
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

bool findIntersection(const line a, const line b, sf::Vector2f* intersection = NULL) {
  return findIntersection(a.first, a.second, b.first, b.second, intersection);
}

bool findIntersection(const sf::Vector2f a_first, const sf::Vector2f a_second, const sf::Vector2f b_first, const sf::Vector2f b_second, sf::Vector2f* intersection = NULL) {
  sf::Vector2f segment_a = a_second - a_first;
  sf::Vector2f segment_b = b_second - b_first;

  float s, t;
  float denominator = (-segment_b.x*segment_a.y + segment_a.x*segment_b.y);

  if(denominator != 0.0f) {
    s = (-segment_a.y*(a_first.x - b_first.x) + segment_a.x*(a_first.y - b_first.y))/denominator;
    t = ( segment_b.x*(a_first.y - b_first.y) - segment_b.y*(a_first.x - b_first.x))/denominator;

    // Test for intersection
    if(s >= 0 && s <= 1 && t >= 0 && t <= 1) {
      if(intersection != NULL) {
	intersection->x = a_first.x + (t*segment_a.x);
	intersection->y = a_first.y + (t*segment_a.y);
      }
      
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

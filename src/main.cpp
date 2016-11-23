#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <vector>
#include <sstream>

#include <SFML/Graphics.hpp>

#define PI 3.1415926535f
#define DEG_TO_RAD PI/180.0f
#define RAD_TO_DEG 180.0f/PI

#define PLAYER_ACCEL 42.0f
#define PLAYER_FRICTION 28.0f
#define PLAYER_MAXSPEED 8.0f
#define PLAYER_LOOK_SENSITIVITY 2.0f
#define PLAYER_TURNRATE 4.5f

#define WINDOW_RESOLUTION_WIDTH 1024
#define WINDOW_RESOLUTION_HEIGHT 768

#define RAYCAST_FOV 60
#define RAYCAST_VIEWDISTANCE 25.0f

using namespace std;

struct entity {
  sf::Vector2f position;
  sf::Vector2f direction;
  sf::Vector2f velocity;

  entity(sf::Vector2f start_position, sf::Vector2f start_direction):
    position(start_position), direction(start_direction) {}

  void rotate(float rotation_angle)
    {
      float new_x, new_y;
      
      new_x = direction.x*cos(-rotation_angle) - direction.y*sin(-rotation_angle);
      new_y = direction.x*sin(-rotation_angle) + direction.y*cos(-rotation_angle);

      direction.x = new_x;
      direction.y = new_y;
    }

  float euler()
    {
      return atan2(direction.y, direction.x);
    }

  float euler_deg()
    {
      return RAD_TO_DEG*this->euler();
    }
};

struct line {
  sf::Vector2f first;
  sf::Vector2f second;

  line(sf::Vector2f first_init, sf::Vector2f second_init):
    first(first_init), second(second_init) {}
};

float magnitude(sf::Vector2f vector);
float sqrMagnitude(sf::Vector2f vector);
sf::Vector2f normalize(const sf::Vector2f input);
bool findIntersection(const line a, const line b, sf::Vector2f* intersection);
bool findIntersection(const sf::Vector2f a_first, const sf::Vector2f a_second, const sf::Vector2f b_first, const sf::Vector2f b_second, sf::Vector2f* intersection);

int main(int argc, char* argv[])
{
  bool paused = false;
  bool mouse_locked = true;

  sf::Clock gameClock;
  
  sf::RenderWindow window(sf::VideoMode(WINDOW_RESOLUTION_WIDTH, WINDOW_RESOLUTION_HEIGHT), "doogue", sf::Style::None);

  // Map Setup
  vector<line> map;
  
  map.push_back(line(sf::Vector2f(-5.0f, -5.0f), sf::Vector2f(-5.0f, 5.0f)));
  map.push_back(line(sf::Vector2f(-5.0f, 5.0f), sf::Vector2f(5.0f, 5.0f)));
  map.push_back(line(sf::Vector2f(5.0f, 5.0f), sf::Vector2f(5.0f, -5.0f)));
  map.push_back(line(sf::Vector2f(5.0f, -5.0f), sf::Vector2f(-5.0f, -5.0f)));

  // Player Setup
  entity player(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 1.0f));
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
      sf::Vector2f player_accel;
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
	player_accel.y += PLAYER_ACCEL*deltaTime.asSeconds();
      }

      if(sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
	player_accel.y -= PLAYER_ACCEL*deltaTime.asSeconds();
      }

      if(sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
	player_accel.x += PLAYER_ACCEL*deltaTime.asSeconds();
      }

      if(sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
	player_accel.x -= PLAYER_ACCEL*deltaTime.asSeconds();
      }

      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
	player.rotate(PLAYER_TURNRATE*deltaTime.asSeconds());
      }

      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
	player.rotate(-PLAYER_TURNRATE*deltaTime.asSeconds());
      }

      // Mouse movement
      sf::Vector2f mouse_current((float)sf::Mouse::getPosition().x, (float)sf::Mouse::getPosition().y);
      sf::Vector2f mouse_delta = mouse_current - mouse_previous;
      mouse_previous = mouse_current;

      if(mouse_locked) {
	sf::Mouse::setPosition(sf::Vector2i(WINDOW_RESOLUTION_WIDTH/2, WINDOW_RESOLUTION_HEIGHT/2), window);
	mouse_previous = sf::Vector2f(sf::Mouse::getPosition().x, sf::Mouse::getPosition().y);
      }

      if(sqrMagnitude(mouse_delta) >= 0.0f) {
	player.rotate(-PLAYER_LOOK_SENSITIVITY*mouse_delta.x*deltaTime.asSeconds());
      }

      // Apply accel to velocity
      player.velocity += player_accel.y*player.direction;
      player.velocity += -player_accel.x*sf::Vector2f(-player.direction.y, player.direction.x);

      // Player physics
      if(sqrMagnitude(player.velocity) > 0.0f) {
	float speed_magnitude = magnitude(player.velocity);

	// Apply friction
	if(speed_magnitude <= PLAYER_FRICTION*deltaTime.asSeconds()) {
	  player.velocity.x = 0.0f;
	  player.velocity.y = 0.0f;
	} else {
	  player.velocity -= (player.velocity/speed_magnitude)*PLAYER_FRICTION*deltaTime.asSeconds();
	}

	// Cap player speed
	// TODO: Replace with proper mathematical approach code
	if(speed_magnitude > PLAYER_MAXSPEED) {
	  player.velocity = PLAYER_MAXSPEED*(player.velocity/speed_magnitude);
	}
	
	// Apply player speed
	player.position += player.velocity*deltaTime.asSeconds();
      }
    }

    // Render game
    window.clear();

    // Raycast walls
    int slice_count = WINDOW_RESOLUTION_WIDTH;
    float slice_width = 1.0f; //(float)RAYCAST_FOV/WINDOW_RESOLUTION_WIDTH;
    sf::Vector2f plane_direction(-player.direction.y, player.direction.x);
    sf::Vector2f plane_anchor = player.position + (WINDOW_RESOLUTION_WIDTH/2.0f)*(float)(1.0f/tan(RAYCAST_FOV))*player.direction;
    sf::Vector2f plane_base = plane_anchor - (WINDOW_RESOLUTION_WIDTH/2.0f)*plane_direction;
    vector<sf::Vertex> slice_points(2*WINDOW_RESOLUTION_WIDTH);

    //*
    for(int slice = 0; slice < WINDOW_RESOLUTION_WIDTH; ++slice) {
      float sqr_distance = -1.0f;

      sf::Vector2f intersection;
      sf::Vector2f slice_target = RAYCAST_VIEWDISTANCE*normalize(plane_base + slice*slice_width*plane_direction);
      line slice_segment(player.position, player.position + slice_target);
      
      for(auto& wall : map) {
	if(findIntersection(slice_segment, wall, &intersection)) {
	  float intersection_distance = sqrMagnitude(intersection - player.position);
	  
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
    player_shape.setPosition(minimap_offset + minimap_scale*player.position - (player_shape.getRadius() + player_shape.getOutlineThickness())*sf::Vector2f(1.0f, 1.0f));
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

sf::Vector2f normalize(const sf::Vector2f input) {
  sf::Vector2f normalized(input.x, input.y);

  return normalized/magnitude(input);
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

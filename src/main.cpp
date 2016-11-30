#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <vector>
#include <sstream>

#include <SFML/Graphics.hpp>

#define PI 3.1415926535f
#define DEG_TO_RAD (PI/180.0f)
#define RAD_TO_DEG (180.0f/PI)

#define PLAYER_ACCEL 42.0f
#define PLAYER_FRICTION 28.0f
#define PLAYER_MAXSPEED 8.0f
#define PLAYER_LOOK_SENSITIVITY 2.0f
#define PLAYER_TURNRATE 1.0f

#define WINDOW_RESOLUTION_WIDTH 1920
#define WINDOW_RESOLUTION_HEIGHT 1080

#define RAYCAST_FOV 60
#define RAYCAST_VIEWDISTANCE 25.0f
#define RAYCAST_RESOLUTION_WIDTH (1920/6)
#define RAYCAST_RESOLUTION_HEIGHT (1080/6)
#define RAYCAST_TEXTURE_SIZE 64
#define RAYCAST_ENTITY_WIDTH 0.06f
#define RAYCAST_SPRITE_WIDTH 64
#define RAYCAST_SPRITE_HEIGHT 72

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

struct render_entity {
  sf::Vector2f position;
  sf::Image sprite;

  render_entity(const sf::Vector2f start_position, const std::string &image_name):
    position(start_position)
    {
      this->sprite.create(RAYCAST_SPRITE_WIDTH, RAYCAST_SPRITE_HEIGHT);
      this->sprite.loadFromFile(image_name);
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

  // Renderer
  double render_depth[RAYCAST_RESOLUTION_WIDTH*RAYCAST_RESOLUTION_HEIGHT];
  sf::Uint8 render_buffer[RAYCAST_RESOLUTION_WIDTH*RAYCAST_RESOLUTION_HEIGHT*4];
  sf::Texture render_texture;
  sf::Sprite render_sprite;
  
  render_texture.create(RAYCAST_RESOLUTION_WIDTH, RAYCAST_RESOLUTION_HEIGHT);
  render_sprite.setTexture(render_texture);

  // Map Setup
  vector<line> map;
  
  map.push_back(line(sf::Vector2f(-5.0f, -5.0f), sf::Vector2f(-5.0f, 5.0f)));
  map.push_back(line(sf::Vector2f(-5.0f, 5.0f), sf::Vector2f(5.0f, 5.0f)));
  map.push_back(line(sf::Vector2f(5.0f, 5.0f), sf::Vector2f(5.0f, -5.0f)));
  map.push_back(line(sf::Vector2f(5.0f, -5.0f), sf::Vector2f(-5.0f, -5.0f)));
  map.push_back(line(sf::Vector2f(1.5f, -0.15f), sf::Vector2f(-1.5f, -0.15f)));
  map.push_back(line(sf::Vector2f(1.5f, 0.15f), sf::Vector2f(-1.5f, 0.15f)));
  map.push_back(line(sf::Vector2f(1.5f, -0.15f), sf::Vector2f(1.5f, 0.15f)));
  map.push_back(line(sf::Vector2f(-1.5f, -0.15f), sf::Vector2f(-1.5f, 0.15f)));

  // Wall Textures
  sf::Image wall_texture;
  wall_texture.create(RAYCAST_TEXTURE_SIZE, RAYCAST_TEXTURE_SIZE);
  wall_texture.loadFromFile("/home/spenser/projects/game-off-2016/build/textures/greystone.png");
  
  // Entity Setup
  vector<render_entity> entities;

  entities.push_back(render_entity(sf::Vector2f(2.5f, 2.5f), "/home/spenser/projects/game-off-2016/build/chest.png"));

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
	switch(event.key.code) {
	case sf::Keyboard::L: mouse_locked = !mouse_locked; break;
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

    // Lazy clear buffer
    // XXX: Should be replaced by some form of smart clear... or not?
    for(int buffer_index = 0; buffer_index < RAYCAST_RESOLUTION_WIDTH*RAYCAST_RESOLUTION_HEIGHT; ++buffer_index) {
      sf::Color backing_color;

      // Choose Color
      if(buffer_index >= (RAYCAST_RESOLUTION_WIDTH*RAYCAST_RESOLUTION_HEIGHT)/2) {
	backing_color = sf::Color(150, 150, 150);
      } else {
	backing_color = sf::Color(40, 40, 40);
      }

      // Clear Pixel
      render_buffer[buffer_index*4 + 0] = backing_color.r;
      render_buffer[buffer_index*4 + 1] = backing_color.g;
      render_buffer[buffer_index*4 + 2] = backing_color.b;
      render_buffer[buffer_index*4 + 3] = backing_color.a;

      // Depth
      render_depth[buffer_index] = 1.0;
    }

    // Raycast walls
    int slice_count = RAYCAST_RESOLUTION_WIDTH;
    float slice_width = 1.0f; //(float)RAYCAST_FOV/RAYCAST_RESOLUTION_WIDTH;
    sf::Vector2f plane_direction(-player.direction.y, player.direction.x);
    sf::Vector2f plane_anchor = player.position + (RAYCAST_RESOLUTION_WIDTH/2.0f)*(float)(1.0f/tan(RAYCAST_FOV))*player.direction;
    sf::Vector2f plane_base = plane_anchor - (RAYCAST_RESOLUTION_WIDTH/2.0f)*plane_direction;
    vector<sf::Vertex> slice_points(2*RAYCAST_RESOLUTION_WIDTH);
    
    for(int slice = 0; slice < RAYCAST_RESOLUTION_WIDTH; ++slice) {
      float textel_offset = 0.0f;
      float sqr_distance = -1.0f;

      sf::Vector2f intersection;
      sf::Vector2f slice_target = RAYCAST_VIEWDISTANCE*normalize(plane_base + slice*slice_width*plane_direction);
      line slice_segment(player.position, player.position + slice_target);

      // Check walls
      for(auto& wall : map) {
	if(findIntersection(slice_segment, wall, &intersection)) {
	  float intersection_distance = sqrMagnitude(intersection - player.position);
	  
	  if(sqr_distance < 0.0f || intersection_distance < sqr_distance) {
	    sqr_distance = intersection_distance;
	    textel_offset = magnitude(intersection - wall.first)/magnitude(wall.second - wall.first);
	  }
	}
      }

      // Draw wall
      if(sqr_distance > 0.0f) {
	float distance = sqrt(sqr_distance);
	float height = RAYCAST_RESOLUTION_HEIGHT/(distance < 0.05f ? 0.05f : distance); // XXX: Hack to avoid division by zero

	int start = (int)(-height/2+RAYCAST_RESOLUTION_HEIGHT/2);
	int end = (int)(height/2+RAYCAST_RESOLUTION_HEIGHT/2);

	int start_capped = start < 0 ? 0 : start;
	int end_capped = end >= RAYCAST_RESOLUTION_HEIGHT ? RAYCAST_RESOLUTION_HEIGHT : end;

	for(int slice_y = start_capped; slice_y < end_capped; ++slice_y) {
	  double local_depth = (double)distance/RAYCAST_VIEWDISTANCE;
	  int render_offset = RAYCAST_RESOLUTION_WIDTH*4*slice_y + slice*4;
	  sf::Color pixel_color = wall_texture.getPixel((int)(16*textel_offset*RAYCAST_TEXTURE_SIZE)%RAYCAST_TEXTURE_SIZE, (int)((slice_y - start_capped + (start < 0 ? -1*start : 0))/height*RAYCAST_TEXTURE_SIZE));

	  if(local_depth < render_depth[RAYCAST_RESOLUTION_WIDTH*slice_y + slice] && pixel_color.a > 0) { // XXX: Dirty transparency fix
	    // Draw pixel
	    // TODO: Appropriately redner pixel transparency
	    render_buffer[render_offset + 0] = pixel_color.r;
	    render_buffer[render_offset + 1] = pixel_color.g;
	    render_buffer[render_offset + 2] = pixel_color.b;
	    render_buffer[render_offset + 3] = pixel_color.a;

	    // Render depth
	    render_depth[RAYCAST_RESOLUTION_WIDTH*slice_y + slice] = local_depth;
	  }
	}
      }

      // Check entities
      for(auto& thing : entities) {
	sf::Vector2f thing_view_direction = slice_segment.second - slice_segment.first;
	line thing_view_segment(thing.position, thing.position); // TODO: Use simple sprite width to make raycast gaurenteed

	// Calculate thing view segment
	thing_view_direction /= magnitude(thing_view_direction);
	float flip_y = -thing_view_direction.y;
	thing_view_direction.y = thing_view_direction.x;
	thing_view_direction.x = flip_y;
	thing_view_segment.first -= RAYCAST_ENTITY_WIDTH*thing_view_direction;
	thing_view_segment.second += RAYCAST_ENTITY_WIDTH*thing_view_direction;
	
	if(findIntersection(slice_segment, thing_view_segment, &intersection)) {
	  if(sqrMagnitude(intersection - player.position) > 0.0f) {
	    float distance = magnitude(intersection - player.position);
	    float distance_scaling = 0.001f;
	    float height = RAYCAST_SPRITE_HEIGHT; //RAYCAST_ENTITY_WIDTH*((float)RAYCAST_SPRITE_HEIGHT/RAYCAST_SPRITE_WIDTH);
	    float thing_textel_u = magnitude(intersection - thing_view_segment.first)/magnitude(thing_view_segment.second - thing_view_segment.first);
	    
	    if(distance > 0.0f) {
	      distance_scaling = 1/distance;
	    }

	    height *= distance_scaling;
	    
	    // Calculate slice height strips
	    int start = (int)(-height/2.0f + RAYCAST_RESOLUTION_HEIGHT/2.0f);
	    int end = (int)(height/2.0f + RAYCAST_RESOLUTION_HEIGHT/2.0f);
	    
	    // Blit sprite into render buffer
	    for(int slice_y = start < 0 ? 0 : start; slice_y < (end >= RAYCAST_RESOLUTION_HEIGHT ? RAYCAST_RESOLUTION_HEIGHT - 1 : end); ++ slice_y) {
	      int render_offset = RAYCAST_RESOLUTION_WIDTH*4*slice_y + 4*slice;
	      float thing_textel_v = (slice_y - start)/height;
	      sf::Color pixel_color = thing.sprite.getPixel(thing_textel_u*RAYCAST_SPRITE_WIDTH, thing_textel_v*RAYCAST_SPRITE_HEIGHT);

	      if(pixel_color.a > 0) {
		// Draw pixel
		render_buffer[render_offset + 0] = pixel_color.r;
		render_buffer[render_offset + 1] = pixel_color.g;
		render_buffer[render_offset + 2] = pixel_color.b;
		render_buffer[render_offset + 3] = pixel_color.a;
	      }
	    }
	  }
	}
      }
    }

    // Update buffer texture
    render_texture.update(&render_buffer[0]);

    // Update the sprite
    render_sprite.setScale((float)WINDOW_RESOLUTION_WIDTH/RAYCAST_RESOLUTION_WIDTH, (float)WINDOW_RESOLUTION_HEIGHT/RAYCAST_RESOLUTION_HEIGHT);
    window.draw(render_sprite);
    
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

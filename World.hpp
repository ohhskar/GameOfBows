#ifndef RC_WORLD
#define RC_WORLD

// SFML Modules
#include <SFML/System/NonCopyable.hpp>

// User Defined
#include "Entities/Character.hpp"
#include "ResourceManager.hpp"
#include "SceneNode.hpp"

class World : private sf::NonCopyable {
 public:
  // Constructors
  explicit World(sf::RenderWindow&);

  // Update and Draw Functions
  void update(sf::Time);
  void draw();

  void handleCollisions();

  // CommandQueue& getCommandQueue();

 private:
  // Enums and Variables
  enum Layer { Background, Ground, Foreground, HUD, LayerCount };

  //World Settings
  sf::RenderWindow& _window;
  sf::View _worldView;
  sf::FloatRect _worldBounds;
  sf::Vector2f _spawnPosition;
  float _scrollSpeed;

  //Scenes
  SceneNode _sceneGraph;
  std::array<SceneNode*, LayerCount> _sceneLayers;

  //Textures
  TextureHolder _textures;
  std::array<std::array<Textures::WallSpecific, 24>, 18> _mapArray;

  Character* _player;
  // CommandQueue mCommandQueue;

  void loadTextures();
  void buildScene();

};
#endif
#include "Character.hpp"
#include <iostream>
Character::Character(Arch arch, unsigned int playerNumber, const TextureHolder& textures)
    : setArrowAim(),
      fireArrow(),
      _playerNumber(playerNumber),
      _archetype(arch),
      _sprite(textures.get(toTextureId(arch)), sf::IntRect(0, 0, 32, 32)),
      _hitbox(sf::Vector2f(40.f, 32.f)),
      _idle(textures.get(toTextureIdAnim(Character::_animationState::Idle))),
      _run(textures.get(toTextureIdAnim(Character::_animationState::Run))),
      _jump(textures.get(toTextureIdAnim(Character::_animationState::Jump))),
      _death(textures.get(toTextureIdAnim(Character::_animationState::Death))),
      _arrowRotation(0.f),
      _arrowPosition(sf::Vector2f(0.f, 0.f)),
      _arrowQuantity(4),
      _aiming(false),
      _firing(false),
      _countdown(sf::Time::Zero),
      _dead(false) {
  // Creating Actions
  sf::FloatRect bounds = _hitbox.getLocalBounds();
  setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
  // _hitbox.setFillColor(sf::Color::White);
  setArrowAim.category = Category::VisualArrow;
  fireArrow.category = Category::ArrowHolder;
  fireArrow.action = [this, &textures](SceneNode& node, sf::Time) { createProjectile(node, textures); };

  // Creating Animations
  _idle.setFrameSize(sf::Vector2i(48, 32));
  _idle.setNumFrames(8);
  _idle.setDuration(sf::seconds(1));
  _idle.setRepeating(true);

  _run.setFrameSize(sf::Vector2i(48, 32));
  _run.setNumFrames(8);
  _run.setDuration(sf::seconds(0.5));
  _run.setRepeating(true);

  _jump.setFrameSize(sf::Vector2i(48, 32));
  _jump.setNumFrames(6);
  _jump.setDuration(sf::seconds(0.5));
  _jump.setRepeating(true);

  _death.setFrameSize(sf::Vector2i(48, 32));
  _death.setNumFrames(11);
  _death.setDuration(sf::seconds(1.5));
}

// Draws and Updates

void Character::updateCurrent(sf::Time dt, CommandQueue& commands) {
  if (getJumping()) {
    _jump.update(dt);
  } else if (getMoving() != 0) {
    _run.update(dt);
  } else {
    _idle.update(dt);
  }
  checkProjectileLaunch(dt, commands);
  MovableEntity::updateCurrent(dt, commands);
}

void Character::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const {
  if (getJumping()) {
    target.draw(_jump, states);
  } else if (getMoving() != 0) {
    target.draw(_run, states);
  } else {
    target.draw(_idle, states);
  }

  // target.draw(_hitbox, states);
}

unsigned int Character::getCategory() const {
  unsigned int collidable =
      getCollidable() == true ? Category::Collidable : Category::Collidable | Category::IgnoreWallCollide;
  switch (_playerNumber) {
    case 1:
      return (Category::PlayerOne | collidable);
    case 2:
      return (Category::PlayerTwo | collidable);
    default:
      return (Category::PlayerOne | collidable);
  }
}

sf::FloatRect Character::getBoundRect() const { return getWorldTransform().transformRect(_hitbox.getGlobalBounds()); }

// Texture Maps
Textures::ID Character::toTextureId(Character::Arch arch) {
  switch (arch) {
    case Character::Arch::Archer:
      return Textures::BlueIdle;
    default:
      return Textures::BlueIdle;
  }
}

Textures::ID Character::toTextureIdAnim(Character::_animationState state) {
  switch (_playerNumber) {
    case 1:
      switch (state) {
        case Idle:
          return Textures::ID::BlueIdle;
        case Run:
          return Textures::ID::BlueRun;
        case Jump:
          return Textures::ID::BlueJump;
        case Death:
          return Textures::ID::BlueDeath;
        default:
          return Textures::ID::BlueIdle;
      }
    case 2:
    default:
      switch (state) {
        case Idle:
          return Textures::ID::PinkIdle;
        case Run:
          return Textures::ID::PinkRun;
        case Jump:
          return Textures::ID::PinkJump;
        case Death:
          return Textures::ID::PinkDeath;
        default:
          return Textures::ID::PinkIdle;
      }
  }
}

// Collision Handling

void Character::handleWallCollision(sf::FloatRect wallBounds) {
  sf::FloatRect ownBounds = getBoundRect();

  float ownBottom = ownBounds.top + ownBounds.height;
  float wallBottom = wallBounds.top + wallBounds.height;
  float ownRight = ownBounds.left + ownBounds.width;
  float wallRight = wallBounds.left + wallBounds.width;

  float bot = wallBottom - ownBounds.top;
  float top = ownBottom - wallBounds.top;
  float left = ownRight - wallBounds.left;
  float right = wallRight - ownBounds.left;
  if (top < bot && top < left && top < right) {
    move(sf::Vector2f(0.f, -top));
    setVelocity(0.f, false);
    setCollidable(false);

  } else if (bot < top && bot < left && bot < right) {
    move(sf::Vector2f(0.f, bot));
    setVelocity(0.f, false);
    // disables wall collisions;
  } else if (left < right && left < top && left < bot) {
    move(sf::Vector2f(-left, 0.f));
    setVelocity(0.f, true);

  } else if (right < left && right < top && right < bot) {
    move(sf::Vector2f(right, 0.f));
    setVelocity(0.f, true);
  }

  // https://stackoverflow.com/questions/5062833/detecting-the-direction-of-a-collision
}

void Character::handleArrowCollision(bool grabbable) {
  if (grabbable) {
    _arrowQuantity++;
  } else {
    _dead = true;
  }
}

// Actions

struct AimArrow {
  // Variable Declerations
  sf::Vector2f position;
  float rotation;

  // Creating Constructor
  AimArrow(sf::Vector2f newPos, float newRot) : position(newPos), rotation(newRot) {}

  // Making the operator
  void operator()(VisualArrow& arrow, sf::Time) const { arrow.aim(position, rotation); }
};

void Character::aim(unsigned int y, unsigned int x, CommandQueue& commands) {
  _aiming = true;
  _arrowRotation = 0.f;
  if (_arrowQuantity > 0) {
    if (x == 1) {
      if (y == 2) {
        _arrowRotation = 135.f;
        _arrowPosition.x = 6.f;
        _arrowPosition.y = 47.f;
      } else if (y == 1) {
        _aiming = true;
        _arrowRotation = 225.f;
        _arrowPosition.x = -10.f;
        _arrowPosition.y = 10.f;
      } else {
        _arrowRotation = 180.f;
        _arrowPosition.x = -5.f;
        _arrowPosition.y = 33.f;
      }
    } else if (x == 2) {
      if (y == 2) {
        _arrowRotation = 45.f;
        _arrowPosition.x = 55.f;
        _arrowPosition.y = 25.f;
      } else if (y == 1) {
        _arrowRotation = -45.f;
        _arrowPosition.x = 32.f;
        _arrowPosition.y = -12.f;
      } else {
        _arrowPosition.x = 45.f;
        _arrowPosition.y = 3.f;
      }
    } else if (x == 0) {
      if (y == 2) {
        _arrowRotation = 90.f;
        _arrowPosition.x = 37.f;
        _arrowPosition.y = 34.f;

      } else if (y == 1) {
        _arrowRotation = -90.f;
        _arrowPosition.x = 4.f;
        _arrowPosition.y = -1.f;
      } else {
        _aiming = false;
        _arrowPosition.x = -1000.f;
        _arrowPosition.y = -1000.f;
      }
    }
  } else {
    _aiming = false;
    _arrowPosition.x = -1000.f;
    _arrowPosition.y = -1000.f;
  }
  // Creating the command
  setArrowAim.action = derivedAction<VisualArrow>(AimArrow(_arrowPosition, _arrowRotation));
  commands.push(setArrowAim);
}

void Character::createProjectile(SceneNode& node, const TextureHolder& textures) const {
  std::unique_ptr<Projectile> projectile(new Projectile(textures, _arrowRotation, _arrowPosition));

  sf::Vector2f finalOffset(getBoundRect().left + _arrowPosition.x, getBoundRect().top + _arrowPosition.y);

  projectile->setPosition(finalOffset);
  node.attachChild(std::move(projectile));
}

void Character::fire() {
  if (_aiming && _arrowQuantity > 0) {
    _firing = true;
  } else {
    _firing = false;
  }
}

void Character::checkProjectileLaunch(sf::Time dt, CommandQueue& commands) {
  if (_firing && _countdown <= sf::Time::Zero) {
    commands.push(fireArrow);
    _countdown += sf::seconds(1.f / 10.f);
    _firing = false;
    _arrowQuantity--;
  } else if (_countdown > sf::Time::Zero) {
    _countdown -= dt;
  }
}
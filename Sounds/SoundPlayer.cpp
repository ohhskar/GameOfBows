#include "SoundPlayer.hpp"


namespace {
// Sound coordinate system, point of view of a player in front of the screen:
// X = left; Y = up; Z = back (out of the screen)
const float ListenerZ = 300.f;
const float Attenuation = 8.f;
const float MinDistance2D = 200.f;
const float MinDistance3D = std::sqrt(MinDistance2D * MinDistance2D + ListenerZ * ListenerZ);
}  // namespace

SoundPlayer::SoundPlayer() : _SoundBuffers(), _Sounds() {
  _SoundBuffers.load(SoundEffect::MenuStart, "../Assets/sfx/start.wav");
  _SoundBuffers.load(SoundEffect::ArrowRecover, "../Assets/sfx/arrowRecover.wav");
  _SoundBuffers.load(SoundEffect::ArrowFire, "../Assets/sfx/fireArrow.wav");
  _SoundBuffers.load(SoundEffect::PlayerJump, "../Assets/sfx/playerJump.wav");
  _SoundBuffers.load(SoundEffect::PlayerDeath, "../Assets/sfx/playerDeath.wav");
  _SoundBuffers.load(SoundEffect::PlayerLand, "../Assets/sfx/playerLand.wav");
  _SoundBuffers.load(SoundEffect::PlayerReady, "../Assets/sfx/playerReady.wav");
  // Listener points towards the screen (default in SFML)
  sf::Listener::setDirection(0.f, 0.f, -1.f);
}

void SoundPlayer::play(SoundEffect::ID effect) { play(effect, getListenerPosition()); }

void SoundPlayer::play(SoundEffect::ID effect, sf::Vector2f position) {
  _Sounds.push_back(sf::Sound());
  sf::Sound& sound = _Sounds.back();

  sound.setBuffer(_SoundBuffers.get(effect));
  sound.setPosition(position.x, -position.y, 0.f);
  sound.setAttenuation(Attenuation);
  sound.setMinDistance(MinDistance3D);

  sound.play();
}

void SoundPlayer::removeStoppedSounds() {
  _Sounds.remove_if([](const sf::Sound& s) { return s.getStatus() == sf::Sound::Stopped; });
}

void SoundPlayer::setListenerPosition(sf::Vector2f position) {
  sf::Listener::setPosition(position.x, -position.y, ListenerZ);
}

sf::Vector2f SoundPlayer::getListenerPosition() const {
  sf::Vector3f position = sf::Listener::getPosition();
  return sf::Vector2f(position.x, -position.y);
}

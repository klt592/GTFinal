#include "Goal.h"

Goal::Goal(Ogre::SceneManager* sceneMgr, std::string name, Ogre::Real x, Ogre::Real y, Ogre::Real z, Ogre::Radian angle) {
  Ogre::Entity* entGoal = sceneMgr->createEntity(name, "geosphere8000.mesh");
  entGoal->setMaterialName("Examples/Aureola", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
  entGoal->setCastShadows(true);
  Ogre::SceneNode* goalNode = sceneMgr->getRootSceneNode()->createChildSceneNode(name, Ogre::Vector3(0,30,0));
  goalNode->attachObject(entGoal);
  goalNode->scale(0.25, 0.25, 0.25);
  goalNode->yaw(angle);
  goalNode->translate(x, y, z);
  setNode(goalNode);
  particle = sceneMgr->createParticleSystem(name + "particle", "Examples/GreenyNimbus");
  particleNode = goalNode->createChildSceneNode(name + "particle");
  particleNode->attachObject(particle);
  particle->fastForward(30);
  particleNode->translate(0, -50, 0);
}

Goal::~Goal(void) {
  Ogre::SceneManager* sceneMgr = particleNode->getCreator();
  sceneMgr->destroyParticleSystem(particle);
  sceneMgr->destroyEntity(particleNode->getName());
  sceneMgr->destroySceneNode(particleNode);
}

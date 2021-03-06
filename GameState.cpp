#include "GameState.h"
 
using namespace Ogre;

CEGUI::Window *face;
CEGUI::Window *life;
CEGUI::Window *health;
CEGUI::Window *pauseWindow;

float hp;
bool showHealth;

CEGUI::MouseButton convertButton2(OIS::MouseButtonID buttonID) {
  switch (buttonID) {
    case OIS::MB_Left:
      return CEGUI::LeftButton;
    case OIS::MB_Right:
      return CEGUI::RightButton;
    case OIS::MB_Middle:
      return CEGUI::MiddleButton;
    default:
      return CEGUI::LeftButton;
  }
}

GameState::GameState() {
  m_bQuit = false;
  m_FrameEvent = Ogre::FrameEvent();
  isPaused = false;
}
 
void GameState::enter() {
  OgreFramework::getSingletonPtr()->m_pLog->logMessage("Entering GameState...");
 
  m_pSceneMgr = OgreFramework::getSingletonPtr()->m_pRoot->createSceneManager(ST_GENERIC, "GameSceneMgr");
  m_pSceneMgr->setAmbientLight(Ogre::ColourValue(0.4f, 0.4f, 0.4f));
  m_pSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
  Ogre::Light* dirLight = m_pSceneMgr->createLight("dirLight");
  dirLight->setType(Ogre::Light::LT_DIRECTIONAL);
  dirLight->setDiffuseColour(Ogre::ColourValue(0.4f, 0.4f, 0.4f));
  dirLight->setSpecularColour(Ogre::ColourValue(0.4f, 0.4f, 0.4f));
  dirLight->setDirection(Ogre::Vector3(0, -1, 0));
 
  m_pCamera = m_pSceneMgr->createCamera("GameCam");
  m_pCamera->setPosition(Vector3(0, 15, -300));
  m_pCamera->lookAt(Vector3(0, 15, 0));
  m_pCamera->setNearClipDistance(1);
 
  m_pCamera->setAspectRatio(Real(OgreFramework::getSingletonPtr()->m_pViewport->getActualWidth()) / Real(OgreFramework::getSingletonPtr()->m_pViewport->getActualHeight()));

  limit = 0.0;

  OgreFramework::getSingletonPtr()->m_pViewport->setCamera(m_pCamera);

  // create an imageset for the face portrait
  CEGUI::ImagesetManager *im = &(CEGUI::ImagesetManager::getSingleton());
  CEGUI::NamedXMLResourceManager<CEGUI::Imageset, CEGUI::Imageset_xmlHandler> *castedIm = NULL;
  castedIm = (CEGUI::NamedXMLResourceManager<CEGUI::Imageset, CEGUI::Imageset_xmlHandler>*) im;
  castedIm->create( "Face.imageset" );

  // hide cursor
  CEGUI::MouseCursor::getSingleton().hide();

  createScene();
}

void GameState::createScene() {
  graphics = new Graphics(m_pSceneMgr);
  physics = new Physics(graphics);
  graphics->setPhysics(physics);
  physics->initialize();
  PenguinNode = m_pSceneMgr->getSceneNode("penguin");

  m_pSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);
  
  // Create a snowstorm
  Ogre::ParticleSystem* snow = m_pSceneMgr->createParticleSystem("snow", "Examples/Snow");
  Ogre::SceneNode* sNode = m_pSceneMgr->getRootSceneNode()->createChildSceneNode("snowNode");
  sNode->translate(0, 1200, 2000);
  sNode->attachObject(snow);
  snow->fastForward(100);

  CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();
  CEGUI::Window *gameWindow = wmgr.createWindow("DefaultWindow", "CEGUI/GameGUI");

  // penguin portrait
  face = CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/StaticImage", "FacePng");
  face->setSize(CEGUI::UVector2(CEGUI::UDim(0.1, 0), CEGUI::UDim(0.13, 0)));
  face->setPosition(CEGUI::UVector2(CEGUI::UDim(0, 0), CEGUI::UDim(0, 0)));
  face->setProperty("Image","set:Face image:full_image");
  face->setProperty("FrameEnabled", "False");
  face->setProperty("BackgroundEnabled", "False");
  gameWindow->addChildWindow(face);

  // life
  life = wmgr.createWindow("TaharezLook/StaticText", "CEGUI/Life");
  stringstream s;
  s << physics->getLives();
  life->setText("x"+s.str());
  life->setSize(CEGUI::UVector2(CEGUI::UDim(0.04, 0), CEGUI::UDim(0.05, 0)));
  life->setPosition(CEGUI::UVector2(CEGUI::UDim(0.08, 0), CEGUI::UDim(0.048, 0)));
  life->setProperty("FrameEnabled", "False");
  life->setProperty("BackgroundEnabled", "False");
  // life->setProperty("Font", "Jura-18");
  gameWindow->addChildWindow(life);

  // health bar for certain stages
  hp = 1.0f;
  showHealth = false;
  health = wmgr.createWindow("TaharezLook/ProgressBar", "CEGUI/Health");
  health->setSize(CEGUI::UVector2(CEGUI::UDim(0.5, 0), CEGUI::UDim(0.04, 0)));
  health->setPosition(CEGUI::UVector2(CEGUI::UDim(0.08, 0), CEGUI::UDim(0.01, 0)));
  health->setProperty("Visible", "False");
  gameWindow->addChildWindow(health);

  // Pause menu
  pauseWindow = wmgr.createWindow("TaharezLook/StaticText", "CEGUI/PauseWindow");
  pauseWindow->setSize(CEGUI::UVector2(CEGUI::UDim(0.4, 0), CEGUI::UDim(0.35, 0)));
  pauseWindow->setPosition(CEGUI::UVector2(CEGUI::UDim(0.3, 0), CEGUI::UDim(0.35, 0)));
  pauseWindow->setProperty("Visible", "False");
  gameWindow->addChildWindow(pauseWindow);

  CEGUI::Window *ret = wmgr.createWindow("TaharezLook/Button", "CEGUI/ReturnToGameButton");
  ret->setText("Resume Game");
  ret->setSize(CEGUI::UVector2(CEGUI::UDim(0.8, 0), CEGUI::UDim(0.2, 0)));
  ret->setPosition(CEGUI::UVector2(CEGUI::UDim(0.1, 0), CEGUI::UDim(0.25, 0)));
  pauseWindow->addChildWindow(ret);

  CEGUI::Window *bac = wmgr.createWindow("TaharezLook/Button", "CEGUI/BackToTitleButton");
  bac->setText("Back to Title");
  bac->setSize(CEGUI::UVector2(CEGUI::UDim(0.8, 0), CEGUI::UDim(0.2, 0)));
  bac->setPosition(CEGUI::UVector2(CEGUI::UDim(0.1, 0), CEGUI::UDim(0.55, 0)));
  pauseWindow->addChildWindow(bac);
  
  CEGUI::System::getSingleton().setGUISheet(gameWindow);

  // event calls
  ret->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&GameState::returnToGame, this));
  bac->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&GameState::backToTitle, this));
}
 
void GameState::UpdateGUI() {
  stringstream s;
  s << physics->getLives();
  life->setProperty("Text", "x"+s.str());

  if(showHealth&&!isPaused){
    hp -= 0.0005;
    if(hp <= 0.0){
      hp = 1.0f;
      physics->penguinOutOfHealth();
    }
    stringstream s2;
    s2 << hp;
    health->setProperty("CurrentProgress", s2.str());
  }
}

void GameState::exit() {
  OgreFramework::getSingletonPtr()->m_pLog->logMessage("Leaving GameState...");
 
  m_pSceneMgr->destroyCamera(m_pCamera);
  if (m_pSceneMgr)
    OgreFramework::getSingletonPtr()->m_pRoot->destroySceneManager(m_pSceneMgr);
 
  /*OgreFramework::getSingletonPtr()->m_pTrayMgr->clearAllTrays();
  OgreFramework::getSingletonPtr()->m_pTrayMgr->destroyAllWidgets();
  OgreFramework::getSingletonPtr()->m_pTrayMgr->setListener(0);*/
}

bool GameState::keyPressed(const OIS::KeyEvent &keyEventRef) {
  if (OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_ESCAPE)) {
    if(isPaused){
      isPaused = false;
      pauseWindow->setProperty("Visible", "False");
      CEGUI::MouseCursor::getSingleton().hide();
    } else {
      isPaused = true;
      pauseWindow->setProperty("Visible", "True");
      CEGUI::MouseCursor::getSingleton().show();
    }
  }

  OgreFramework::getSingletonPtr()->keyPressed(keyEventRef);
  CEGUI::System &sys = CEGUI::System::getSingleton();
  sys.injectKeyDown(keyEventRef.key);
  sys.injectChar(keyEventRef.text);
  return true;
}
 
bool GameState::keyReleased(const OIS::KeyEvent &keyEventRef) {
  OgreFramework::getSingletonPtr()->keyReleased(keyEventRef);
  //CEGUI::System::getSingleton().injectKeyUp(keyEventRef.key);
  return true;
}
 
bool GameState::mouseMoved(const OIS::MouseEvent &evt) {
  CEGUI::System &sys = CEGUI::System::getSingleton();
  sys.injectMouseMove(evt.state.X.rel, evt.state.Y.rel);
  if(!isPaused){
    physics->rotate(0, Ogre::Degree(-evt.state.X.rel * 0.1).valueRadians());
  
    if((evt.state.Y.rel > 0 && limit <= 360) || (evt.state.Y.rel < 0 && limit >= -30)){
      limit += evt.state.Y.rel;
        m_pCamera->move(Ogre::Vector3(0.0,evt.state.Y.rel,0.0));
        m_pCamera->pitch(Ogre::Degree(-evt.state.Y.rel*0.1));
    }
  }
  // Scroll wheel.
  if (evt.state.Z.rel)
    sys.injectMouseWheelChange(evt.state.Z.rel / 120.0f);
  // if(OgreFramework::getSingletonPtr()->m_pTrayMgr->injectMouseMove(evt)) return true;
  return true;
}
 
bool GameState::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id) {
  CEGUI::System::getSingleton().injectMouseButtonDown(convertButton2(id));
  // if(OgreFramework::getSingletonPtr()->m_pTrayMgr->injectMouseDown(evt, id)) return true;
  return true;
}
 
bool GameState::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id) {
  CEGUI::System::getSingleton().injectMouseButtonUp(convertButton2(id));
  // if(OgreFramework::getSingletonPtr()->m_pTrayMgr->injectMouseUp(evt, id)) return true;
  return true;
}

void GameState::update(double timeSinceLastFrame) {
  m_FrameEvent.timeSinceLastFrame = timeSinceLastFrame;
  CEGUI::System::getSingleton().injectTimePulse(m_FrameEvent.timeSinceLastFrame);
  // OgreFramework::getSingletonPtr()->m_pTrayMgr->frameRenderingQueued(m_FrameEvent);
  getInput();
  physics->step();
  if (m_bQuit == true) {
    shutdown();
    return;
  }
  if (physics->getStage() == 1) {
    health->setProperty("Visible", "True");
    showHealth = true;
  } else{
    health->setProperty("Visible", "False");
    showHealth = false;
  }
  UpdateGUI();
  if (physics->gameOver()) {
    CEGUI::WindowManager::getSingleton().destroyWindow( "CEGUI/GameGUI" );
    CEGUI::WindowManager::getSingleton().destroyWindow( "CEGUI/PauseWindow" );
    popAllAndPushAppState(findByName("GameOverState"));
  }
}

void GameState::getInput() {
if(!isPaused){
  OIS::Keyboard* keyboard = OgreFramework::getSingletonPtr()->m_pKeyboard;

  if (keyboard->isKeyDown(OIS::KC_Q)) {
    physics->rotate(0, Ogre::Degree(3).valueRadians());
  }
  if (keyboard->isKeyDown(OIS::KC_E)) {
    physics->rotate(0, Ogre::Degree(-3).valueRadians());
  }
  Ogre::Vector3 dir = PenguinNode->getOrientation() * Ogre::Vector3::UNIT_Z;
  if (keyboard->isKeyDown(OIS::KC_W) || keyboard->isKeyDown(OIS::KC_UP)) {
    physics->translate(0, 5.0f * dir[0], 0.0f, 5.0f * dir[2]);
  }
  if (keyboard->isKeyDown(OIS::KC_S) || keyboard->isKeyDown(OIS::KC_DOWN)) {
    physics->translate(0, -5.0f * dir[0], 0.0f, -5.0f * dir[2]);
  }
  if (keyboard->isKeyDown(OIS::KC_A) || keyboard->isKeyDown(OIS::KC_LEFT)) {
    physics->translate(0, 5.0f * dir[2], 0.0f, -5.0f * dir[0]);
  }
  if (keyboard->isKeyDown(OIS::KC_D) || keyboard->isKeyDown(OIS::KC_RIGHT)) {
    physics->translate(0, -5.0f * dir[2], 0.0f, 5.0f * dir[0]);
  }

  if (keyboard->isKeyDown(OIS::KC_SPACE)) {
    if(!graphics->getJumping() && physics->has_jumping()){
      physics->applyForce(0, 0, 14000, 0);
      graphics->playSound(0);
      graphics->setJumping(true);
    }
  }
}
}

bool GameState::returnToGame(const CEGUI::EventArgs &e) {
  isPaused = false;
  pauseWindow->setProperty("Visible", "False");
  CEGUI::MouseCursor::getSingleton().hide();
}

bool GameState::backToTitle(const CEGUI::EventArgs &e) {
  isPaused = false;
  CEGUI::WindowManager::getSingleton().destroyWindow( "CEGUI/GameGUI" );
  CEGUI::WindowManager::getSingleton().destroyWindow( "CEGUI/PauseWindow" );
  popAllAndPushAppState(findByName("MenuState"));
}

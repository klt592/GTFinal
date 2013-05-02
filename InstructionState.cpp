#include "InstructionState.h"
 
using namespace Ogre;

CEGUI::MouseButton convertButton3(OIS::MouseButtonID buttonID) {
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

InstructionState::InstructionState() {
  m_bQuit = false;
  m_FrameEvent = Ogre::FrameEvent();
}
 
void InstructionState::enter() {
  OgreFramework::getSingletonPtr()->m_pLog->logMessage("Entering InstructionState...");
 
  m_pSceneMgr = OgreFramework::getSingletonPtr()->m_pRoot->createSceneManager(ST_GENERIC, "InstructionSceneMgr");
  m_pSceneMgr->setAmbientLight(Ogre::ColourValue(0.7f, 0.7f, 0.7f));
 
  m_pCamera = m_pSceneMgr->createCamera("InstructCam");
  m_pCamera->setPosition(Vector3(0, 25, -50));
  m_pCamera->lookAt(Vector3(0, 0, 0));
  m_pCamera->setNearClipDistance(1);
 
  m_pCamera->setAspectRatio(Real(OgreFramework::getSingletonPtr()->m_pViewport->getActualWidth()) / Real(OgreFramework::getSingletonPtr()->m_pViewport->getActualHeight()));
 
  OgreFramework::getSingletonPtr()->m_pViewport->setCamera(m_pCamera);

  createScene();
}

void InstructionState::createScene() {
  CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();
  CEGUI::Window *window = wmgr.createWindow("DefaultWindow", "CEGUI/InstructionWindow");

  // instructions
  CEGUI::Window *instructions = wmgr.createWindow("TaharezLook/StaticText", "CEGUI/Instructions");
  instructions->setText("Notice: This is a temporary UI, will update if we have time\n \n \n \n \nControls:\n \nMove forward with W or up arrow\nMove backward with S or down arrow\nStraft left with A or left arrow\nStraft right with D or right arrow\nTurn left with Q\nTurn right with R\nJump with Spacebar");
  instructions->setSize(CEGUI::UVector2(CEGUI::UDim(0.8, 0), CEGUI::UDim(0.8, 0)));
  instructions->setPosition(CEGUI::UVector2(CEGUI::UDim(0.1, 0), CEGUI::UDim(0.05, 0)));
  window->addChildWindow(instructions);

  // back to title button
  CEGUI::Window *back_to_title = wmgr.createWindow("TaharezLook/Button", "CEGUI/backToTitleButton");
  back_to_title->setText("Back");
  back_to_title->setSize(CEGUI::UVector2(CEGUI::UDim(0.1, 0), CEGUI::UDim(0.05, 0)));
  back_to_title->setPosition(CEGUI::UVector2(CEGUI::UDim(0.8, 0), CEGUI::UDim(0.9, 0)));
  window->addChildWindow(back_to_title);

  CEGUI::System::getSingleton().setGUISheet(window);

  // event calls
  back_to_title->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&InstructionState::backToTitle, this));
}
 
void InstructionState::exit() {
  OgreFramework::getSingletonPtr()->m_pLog->logMessage("Leaving InstructionState...");
 
  m_pSceneMgr->destroyCamera(m_pCamera);
  if (m_pSceneMgr)
    OgreFramework::getSingletonPtr()->m_pRoot->destroySceneManager(m_pSceneMgr);
 
  /*OgreFramework::getSingletonPtr()->m_pTrayMgr->clearAllTrays();
  OgreFramework::getSingletonPtr()->m_pTrayMgr->destroyAllWidgets();
  OgreFramework::getSingletonPtr()->m_pTrayMgr->setListener(0);*/
}

bool InstructionState::keyPressed(const OIS::KeyEvent &keyEventRef) {
  if (OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_ESCAPE)) {
    CEGUI::WindowManager::getSingleton().destroyWindow( "CEGUI/InstructionWindow" );
    popAllAndPushAppState(findByName("MenuState"));
    return true;
  }
 
  //OgreFramework::getSingletonPtr()->keyPressed(keyEventRef);
  CEGUI::System &sys = CEGUI::System::getSingleton();
  sys.injectKeyDown(keyEventRef.key);
  sys.injectChar(keyEventRef.text);
  return true;
}
 
bool InstructionState::keyReleased(const OIS::KeyEvent &keyEventRef) {
  //OgreFramework::getSingletonPtr()->keyReleased(keyEventRef);
  CEGUI::System::getSingleton().injectKeyUp(keyEventRef.key);
  return true;
}
 
bool InstructionState::mouseMoved(const OIS::MouseEvent &evt) {
  CEGUI::System &sys = CEGUI::System::getSingleton();
  sys.injectMouseMove(evt.state.X.rel, evt.state.Y.rel);
  // Scroll wheel.
  if (evt.state.Z.rel)
    sys.injectMouseWheelChange(evt.state.Z.rel / 120.0f);
  // if(OgreFramework::getSingletonPtr()->m_pTrayMgr->injectMouseMove(evt)) return true;
  return true;
}
 
bool InstructionState::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id) {
  CEGUI::System::getSingleton().injectMouseButtonDown(convertButton3(id));
  // if(OgreFramework::getSingletonPtr()->m_pTrayMgr->injectMouseDown(evt, id)) return true;
  return true;
}
 
bool InstructionState::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id) {
  CEGUI::System::getSingleton().injectMouseButtonUp(convertButton3(id));
  // if(OgreFramework::getSingletonPtr()->m_pTrayMgr->injectMouseUp(evt, id)) return true;
  return true;
}

void InstructionState::update(double timeSinceLastFrame) {
  m_FrameEvent.timeSinceLastFrame = timeSinceLastFrame;
  CEGUI::System::getSingleton().injectTimePulse(m_FrameEvent.timeSinceLastFrame);
  // OgreFramework::getSingletonPtr()->m_pTrayMgr->frameRenderingQueued(m_FrameEvent);
}

bool InstructionState::backToTitle(const CEGUI::EventArgs &e) {
  CEGUI::WindowManager::getSingleton().destroyWindow( "CEGUI/InstructionWindow" );
  changeAppState(findByName("MenuState"));
}
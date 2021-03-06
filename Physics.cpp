#include "Physics.h"

btVector3 checkpoint;

#define BIT(x) (1<<(x))
enum collisiontypes {
  COL_NOTHING = 0, //<Collide with nothing
  COL_PENGUIN = BIT(0), //<Collide with penguins
  COL_WALL = BIT(1), //<Collide with walls
  COL_GROUND = BIT(2), //<Collide with grounds
  COL_KILLBOX = BIT(3), //<Collide with killboxes
  COL_GOAL = BIT(4), //<Collide with goal
  COL_CHECKPOINT = BIT(5) //<Collid with checkpoint
};

enum propertytypes {
  NONE = 0,
  CHECKPOINT = BIT(0),
  KILLBOX = BIT(1),
  INVISIBLE = BIT(2)
};

Physics::Physics(Graphics* graphic) {
  collisionConfiguration = new btDefaultCollisionConfiguration();
  dispatcher = new btCollisionDispatcher(collisionConfiguration);
  overlappingPairCache = new btDbvtBroadphase();
  solver = new btSequentialImpulseConstraintSolver();
  dynamicWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
  dynamicWorld->setGravity(btVector3(0,-300,0));
  dynamicWorld->getSolverInfo().m_splitImpulse = true;
  graphics = graphic;
  stageNumber = 0;
  lives = 5;
  jumping = true;
}

Physics::~Physics(void) {
  delete dynamicWorld;
}

void Physics::step(void) {
  dynamicWorld->stepSimulation(1.f/60.f);
  graphics->animate();
  int numManifolds = dynamicWorld->getDispatcher()->getNumManifolds();
  for (int i = 0; i < numManifolds; i++) {
    btPersistentManifold* contactManifold =  dynamicWorld->getDispatcher()->getManifoldByIndexInternal(i);
    btCollisionObject* obA = static_cast<btCollisionObject*>(contactManifold->getBody0());
    btCollisionObject* obB = static_cast<btCollisionObject*>(contactManifold->getBody1());
    btBroadphaseProxy* obA_proxy = obA->getBroadphaseHandle();
    btBroadphaseProxy* obB_proxy = obB->getBroadphaseHandle();
    if (obA_proxy->m_collisionFilterGroup & obB_proxy->m_collisionFilterMask) {
      if (obA_proxy->m_collisionFilterGroup == COL_PENGUIN && obB_proxy->m_collisionFilterGroup == COL_KILLBOX) {
        PhysicsBody* object = reinterpret_cast<PhysicsBody*>(obA->getUserPointer());
        resetObject(object);
      } else if (obA_proxy->m_collisionFilterGroup == COL_PENGUIN && obB_proxy->m_collisionFilterGroup == COL_GOAL) {
        nextStage();
        graphics->playSound(2);
      } else if (obA_proxy->m_collisionFilterGroup == COL_PENGUIN && obB_proxy->m_collisionFilterGroup == COL_CHECKPOINT) {
        btVector3 checkpoint2 = obB->getWorldTransform().getOrigin();
        btBoxShape* shape = reinterpret_cast<btBoxShape*>(obB->getCollisionShape());
        checkpoint2 += btVector3(0, shape->getHalfExtentsWithoutMargin().y(), 0);
        checkpoint2 += btVector3(0, 25, 0);
        PhysicsBody* object = reinterpret_cast<PhysicsBody*>(obA->getUserPointer());
        btRigidBody* body = object->getBody();
        btVector3 translate = body->getCenterOfMassPosition();
        if (checkpoint2 != checkpoint && translate.y() >= checkpoint2.y()) {
          graphics->playSound(1);
          checkpoint = checkpoint2;
        }
      }
    }
  }
}

void Physics::resetObject(PhysicsBody* object) {
  btRigidBody* body = object->getBody();
  btVector3 translate = body->getCenterOfMassPosition();
  btVector3 diff = translate - checkpoint;
  body->setLinearVelocity(btVector3(0,0,0));
  body->setAngularVelocity(btVector3(0,0,0));
  if (diff.x() < 0.001 && diff.x() > -.001 && diff.y() == 0 && diff.z() == 5)
    return;
  body->translate(-translate + checkpoint);
  --lives;
}

void Physics::initialize(void) {
  graphics->loadSounds();
  addPenguin("penguin");
  addStage0();
}

void Physics::nextStage(void) {
  removeStage();
  ++stageNumber;
  if (stageNumber == 0) {
    addStage0();
  } else if (stageNumber == 1) {
    addStage2();
  } else if (stageNumber == 2) {
    addStage1();
  } else {
    lives = -1;
    stageNumber = 0;
    addStage0();
  }
}

void Physics::addStage0(void) {
  addGround("ground", 0, 0, 0, 0, 8000, 0, 8000, KILLBOX);
  addGoal("goal", -60, 845, 3640, 0);
  addWall("wall0", 0, 440, 0, 0, 80, 40, 120);   // start
  addWall("wall1", 0, 520, 160, 0, 80, 40, 40);  // stairs 1
  addWall("wall2", 0, 600, 240, 0, 80, 40, 40);
  addWall("wall3", 0, 680, 360, 0, 80, 40, 80);
  addWall("wall4", 0, 680, 1080, 0, 240, 40, 240);  // platform 1
  addWall("wall5", -520, 600, 1160, 0, 40, 40, 80);
  addWall("wall6", -760, 400, 1640, 0, 40, 80, 40);
  addWall("wall7", -1080, 480, 2000, 0, 80, 20, 80);
  addWall("wall8", -920, 560, 2160, 0, 20, 20, 120);
  addWall("wall9", -800, 640, 2280, 0, 40, 30, 40);
  addWall("wall10", -880, 720, 1880, 0, 40, 20, 80);
  addWall("wall11", -880, 780, 1760, 0, 40, 40, 40);
  addWall("wall12", -720, 880, 2200, 45, 80, 20, 320, CHECKPOINT); // end of jump puzzle 1
  addWall("wall12_2", -230, 940, 2200, 0, 15, 10, 40);
  addWall("wall12_3", -170, 1020, 2140, 0, 15, 10, 40);
  addWall("wall12_4", -110, 1090, 2080, 0, 15, 10, 40);
  addWall("wall12_5", -50, 1160, 2210, 0, 15, 10, 40, CHECKPOINT);
  addWall("wall12_6", -10, 900, 1880, 0, 15, 10, 40);
  addWall("wall12_7", -40, 750, 1480, 0, 50, 5, 50);
  addWall("wall12_8", 30, 1250, 2280, 0, 32, 20, 25);
  addWall("wall12_9", 60, 1450, 2500, 0, 80, 20, 50);
  addWall("wall13", -160, 715, 3240, 0, 120, 10, 120); // platform on other side of wall
  addWall("wall14", 96, 390, 2690, 0, 804, 400, 50);   // Big Wall Bottom
  addWall("wall15", 100, 1190, 2680, 0, 800, 400, 40); // Big Wall Top
  addWall("wall16", -60, 795, 3640, 0, 100, 10, 100); // End?
  jumping = true;
}

void Physics::addStage1(void) {
  addGround("ground", 0, 0, 0, 0, 8000, 0, 8000, KILLBOX);
  addWall("south0", 0, 440, 0, 0, 760, 40, 120);   // start
  addWall("wall1", 0, 520, 160, 0, 80, 40, 40);  // stairs 1
  addWall("wall2", 0, 600, 240, 0, 80, 40, 40);
  addWall("wall3", 0, 680, 360, 0, 80, 40, 80);
  addWall("wall4", -220, 700, 540, 0, 40, 10, 40);
  addWall("wall5", -540, 740, 460, 0, 40, 10, 40);
  addWall("wall6", -320, 820, 380, 0, 40, 10, 40);
  addWall("wall7", -20, 860, 370, 0, 40, 10, 40);
  addWall("wall8", 420, 840, 420, 0, 40, 10, 40);
  addWall("wall9", 820, 900, 420, 0, 40, 10, 40);
  addWall("west0", 880, 1030, 800, 0, 120, 40, 670); 
  addWall("wall10", 1340, 780, 660, 0, 40, 10, 40);
  addWall("wall11", 1820, 700, 320, 0, 40, 10, 40);
  addWall("wall12", 2060, 760, 400, 0, 40, 10, 40);
  addWall("wall12_2", 2060, 760, 800, 0, 40, 10, 40);
  addWall("wall13", 1810, 780, 1200, 0, 40, 10, 60);
  addWall("wall14", 1380, 800, 1300, 0, 40, 40, 80);  // stairs 2
  addWall("wall15", 1260, 880, 1300, 0, 40, 40, 80);
  addWall("wall16", 1140, 970, 1300, 0, 40, 40, 80);
  addWall("wall17", 170, 1100, 1000, 0, 280, 20, 100, CHECKPOINT);
  addWall("east0", -880, 840, 800, 0, 120, 40, 670);
  addWall("wall18", -960, 960, 300, 0, 20, 10, 20);
  addWall("wall19", -840, 1010, 370, 0, 20, 10, 20);
  addWall("wall20", -760, 1040, 410, 0, 20, 10, 20);
  addWall("wall21", -540, 1100, 500, 0, 20, 10, 20);
  addWall("wall22", -540, 1140, 620, 0, 20, 10, 20);
  addWall("wall23", -540, 1210, 500, 0, 20, 10, 20);
  addWall("wall24", -580, 1280, 300, 0, 20, 10, 20);
  addWall("wall25", -600, 1300, 600, 0, 20, 10, 20);  // Stepping stone
  addWall("wall26", -620, 1350, 680, 0, 10, 10, 10);
  addWall("wall27", -580, 1355, 720, 0, 10, 10, 10);
  addWall("wall28", -580, 1360, 1120, 0, 10, 10, 10);
  addWall("wall29", -250, 1370, 1060, 0, 10, 10, 10);
  addWall("wall30", 0, 1400, 1000, 0, 15, 10, 15);
  addWall("wall31", -100, 1460, 1300, 0, 35, 10, 35);
  addWall("wall32", 0, 1460, 1600, 0, 28, 10, 28);
  addWall("wall33", 100, 1460, 1300, 0, 35, 10, 35);
  addWall("wall34", 0, 1500, 1950, 0, 100, 10, 40, CHECKPOINT);
  addWall("wall35", 0, 1560, 2300, 0, 40, 10, 40);
  addWall("north0", 0, 2360, 1600, 0, 750, 800, 120);  // great wall climb
  addWall("wall36", 300, 1640, 2200, 0, 80, 15, 40);
  addWall("wall37", 500, 1670, 1750, 0, 60, 15, 30);
  addWall("wall38", 100, 1700, 1750, 0, 60, 15, 30);
  addWall("wall39", -50, 1770, 1750, 0, 60, 10, 30);
  addWall("wall40", 150, 1835, 1750, 0, 70, 15, 30);
  addWall("wall41", 300, 1900, 1750, 0, 70, 15, 30);
  addWall("wall42", 450, 1970, 1750, 0, 50, 15, 30);
  addWall("wall43", 600, 2140, 1750, 0, 60, 15, 30);
  addWall("wall44", 350, 2200, 1750, 0, 70, 15, 30);
  addWall("wall45", 0, 2000, 1750, 0, 40, 15, 30);
  addWall("wall46", -180, 2080, 1750, 0, 40, 15, 30);
  addWall("wall47", 180, 2130, 1750, 0, 60, 15, 30);
  addWall("wall48", 980, 2220, 1750, 0, 70, 10, 30);
  addWall("wall49", 700, 2300, 1750, 0, 70, 10, 30);
  addWall("wall50", 300, 2360, 1750, 0, 70, 10, 30);
  addWall("wall51", 0, 2420, 1750, 0, 70, 10, 30);
  addWall("wall52", -300, 2490, 1750, 0, 70, 10, 30);
  addWall("wall53", -600, 2545, 1750, 0, 70, 10, 30);
  addWall("wall54", -900, 2620, 1750, 0, 70, 10, 30);
  addWall("wall55", -750, 2700, 1750, 0, 70, 10, 30);
  addWall("wall56", -1050, 2780, 1650, 0, 70, 10, 30, CHECKPOINT);
  addWall("wall57", -1050, 2840, 1400, 0, 40, 10, 40);
  addWall("wall58", -1050, 2920, 1200, 0, 40, 10, 40);
  addWall("wall59", -1050, 3000, 1100, 0, 40, 10, 40);
  addWall("wall60", -800, 3080, 1000, 0, 40, 10, 40);
  addGoal("goal", -400, 2800, 1000, 0);
  addWall("pillarsw", 880, 2000, 0, 0, 120, 1600, 120);
  addWall("pillarnw", 880, 2000, 1600, 0, 120, 1600, 120);
  addWall("pillarne", -880, 2000, 1600, 0, 120, 1250, 120);
  addWall("pillarse", -880, 2000, 0, 0, 120, 1600, 120);
  movePenguin(btVector3(0, 505, 0));
  checkpoint = btVector3(0, 505, 0);
  jumping = true;
}

void Physics::addStage2(void) {
  addGround("ground", 0, 0, 0, 0, 8000, 0, 8000);
  addGoal("goal", 50, 25, 1000, 0);
  addWall("barrior_s", 0, 60, -200, 0, 500, 60, 50);
  addWall("barrior_n", 0, 60, 1200, 0, 500, 60, 50);
  addWall("barrior_w", 550, 60, 500, SIMD_HALF_PI, 650, 60, 50);
  addWall("barrior_e", -550, 60, 500, SIMD_HALF_PI, 650, 60, 50);

//   addWall("wall0", 450, 60, -100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall1", 350, 60, -100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall2", 250, 60, -100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall3", 150, 60, -100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall4", 50, 60, -100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall5", -50, 60, -100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall6", -150, 60, -100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall7", -250, 60, -100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall8", -350, 60, -100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall9", -450, 60, -100, 0, 50, 60, 50, INVISIBLE);


//   addWall("wall10", 450, 60, 0, 0, 50, 60, 50, INVISIBLE);
  addWall("wall11", 350, 60, 0, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall12", 250, 60, 0, 0, 50, 60, 50, INVISIBLE);
  addWall("wall13", 150, 60, 0, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall14", 50, 60, 0, 0, 50, 60, 50, INVISIBLE);
  addWall("wall15", -50, 60, 0, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall16", -150, 60, 0, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall17", -250, 60, 0, 0, 50, 60, 50, INVISIBLE);
  addWall("wall18", -350, 60, 0, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall19", -450, 60, 0, 0, 50, 60, 50, INVISIBLE);

//   addWall("wall20", 450, 60, 100, 0, 50, 60, 50, INVISIBLE);
  addWall("wall21", 350, 60, 100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall22", 250, 60, 100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall23", 150, 60, 100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall24", 50, 60, 100, 0, 50, 60, 50, INVISIBLE);
  addWall("wall25", -50, 60, 100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall26", -150, 60, 100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall27", -250, 60, 100, 0, 50, 60, 50, INVISIBLE);
  addWall("wall28", -350, 60, 100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall29", -450, 60, 100, 0, 50, 60, 50, INVISIBLE);

  addWall("wall30", 450, 60, 200, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall31", 350, 60, 200, 0, 50, 60, 50, INVISIBLE);
  addWall("wall32", 250, 60, 200, 0, 50, 60, 50, INVISIBLE);
  addWall("wall33", 150, 60, 200, 0, 50, 60, 50, INVISIBLE);
  addWall("wall34", 50, 60, 200, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall35", -50, 60, 200, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall36", -150, 60, 200, 0, 50, 60, 50, INVISIBLE);
  addWall("wall37", -250, 60, 200, 0, 50, 60, 50, INVISIBLE);
  addWall("wall38", -350, 60, 200, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall39", -450, 60, 200, 0, 50, 60, 50, INVISIBLE);

//   addWall("wall40", 450, 60, 300, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall41", 350, 60, 300, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall42", 250, 60, 300, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall43", 150, 60, 300, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall44", 50, 60, 300, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall45", -50, 60, 300, 0, 50, 60, 50, INVISIBLE);
  addWall("wall46", -150, 60, 300, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall47", -250, 60, 300, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall48", -350, 60, 300, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall49", -450, 60, 300, 0, 50, 60, 50, INVISIBLE);

//   addWall("wall50", 450, 60, 400, 0, 50, 60, 50, INVISIBLE);
  addWall("wall51", 350, 60, 400, 0, 50, 60, 50, INVISIBLE);
  addWall("wall52", 250, 60, 400, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall53", 150, 60, 400, 0, 50, 60, 50, INVISIBLE);
  addWall("wall54", 50, 60, 400, 0, 50, 60, 50, INVISIBLE);
  addWall("wall55", -50, 60, 400, 0, 50, 60, 50, INVISIBLE);
  addWall("wall56", -150, 60, 400, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall57", -250, 60, 400, 0, 50, 60, 50, INVISIBLE);
  addWall("wall58", -350, 60, 400, 0, 50, 60, 50, INVISIBLE);
  addWall("wall59", -450, 60, 400, 0, 50, 60, 50, INVISIBLE);

//   addWall("wall60", 450, 60, 500, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall61", 350, 60, 500, 0, 50, 60, 50, INVISIBLE);
  addWall("wall62", 250, 60, 500, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall63", 150, 60, 500, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall64", 50, 60, 500, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall65", -50, 60, 500, 0, 50, 60, 50, INVISIBLE);
  addWall("wall66", -150, 60, 500, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall67", -250, 60, 500, 0, 50, 60, 50, INVISIBLE);
  addWall("wall68", -350, 60, 500, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall69", -450, 60, 500, 0, 50, 60, 50, INVISIBLE);

  addWall("wall70", 450, 60, 600, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall71", 350, 60, 600, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall72", 250, 60, 600, 0, 50, 60, 50, INVISIBLE);
  addWall("wall73", 150, 60, 600, 0, 50, 60, 50, INVISIBLE);
  addWall("wall74", 50, 60, 600, 0, 50, 60, 50, INVISIBLE);
  addWall("wall75", -50, 60, 600, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall76", -150, 60, 600, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall77", -250, 60, 600, 0, 50, 60, 50, INVISIBLE);
  addWall("wall78", -350, 60, 600, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall79", -450, 60, 600, 0, 50, 60, 50, INVISIBLE);

//   addWall("wall80", 450, 60, 700, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall81", 350, 60, 700, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall82", 250, 60, 700, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall83", 150, 60, 700, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall84", 50, 60, 700, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall85", -50, 60, 700, 0, 50, 60, 50, INVISIBLE);
  addWall("wall86", -150, 60, 700, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall87", -250, 60, 700, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall88", -350, 60, 700, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall89", -450, 60, 700, 0, 50, 60, 50, INVISIBLE);

//   addWall("wall90", 450, 60, 800, 0, 50, 60, 50, INVISIBLE);
  addWall("wall91", 350, 60, 800, 0, 50, 60, 50, INVISIBLE);
  addWall("wall92", 250, 60, 800, 0, 50, 60, 50, INVISIBLE);
  addWall("wall93", 150, 60, 800, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall94", 50, 60, 800, 0, 50, 60, 50, INVISIBLE);
  addWall("wall95", -50, 60, 800, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall96", -150, 60, 800, 0, 50, 60, 50, INVISIBLE);
  addWall("wall97", -250, 60, 800, 0, 50, 60, 50, INVISIBLE);
  addWall("wall98", -350, 60, 800, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall99", -450, 60, 800, 0, 50, 60, 50, INVISIBLE);

//   addWall("wall100", 450, 60, 900, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall101", 350, 60, 900, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall102", 250, 60, 900, 0, 50, 60, 50, INVISIBLE);
  addWall("wall103", 150, 60, 900, 0, 50, 60, 50, INVISIBLE);
  addWall("wall104", 50, 60, 900, 0, 50, 60, 50, INVISIBLE);
  addWall("wall105", -50, 60, 900, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall106", -150, 60, 900, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall107", -250, 60, 900, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall108", -350, 60, 900, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall109", -450, 60, 900, 0, 50, 60, 50, INVISIBLE);

//   addWall("wall110", 450, 60, 1000, 0, 50, 60, 50, INVISIBLE);
  addWall("wall111", 350, 60, 1000, 0, 50, 60, 50, INVISIBLE);
  addWall("wall112", 250, 60, 1000, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall113", 150, 60, 1000, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall114", 50, 60, 1000, 0, 50, 60, 50, INVISIBLE);
  addWall("wall115", -50, 60, 1000, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall116", -150, 60, 1000, 0, 50, 60, 50, INVISIBLE);
  addWall("wall117", -250, 60, 1000, 0, 50, 60, 50, INVISIBLE);
  addWall("wall118", -350, 60, 1000, 0, 50, 60, 50, INVISIBLE);
  addWall("wall119", -450, 60, 1000, 0, 50, 60, 50, INVISIBLE);

//   addWall("wall120", 450, 60, 1100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall121", 350, 60, 1100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall122", 250, 60, 1100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall123", 150, 60, 1100, 0, 50, 60, 50, INVISIBLE);
  addWall("wall124", 50, 60, 1100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall125", -50, 60, 1100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall126", -150, 60, 1100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall127", -250, 60, 1100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall128", -350, 60, 1100, 0, 50, 60, 50, INVISIBLE);
//   addWall("wall129", -450, 60, 1100, 0, 50, 60, 50, INVISIBLE);

  movePenguin(btVector3(25, 25, -100));
  checkpoint = btVector3(25, 25, -100);
  jumping = false;
}

void Physics::movePenguin(btVector3 location) {
  btRigidBody* body = gameBodies.at(0)->getBody();
  btVector3 translate = body->getCenterOfMassPosition();
  body->translate(-translate + location);
  body->setLinearVelocity(btVector3(0,0,0));
  body->setAngularVelocity(btVector3(0,0,0));
}

void Physics::penguinOutOfHealth(void) {
  btRigidBody* body = gameBodies.at(0)->getBody();
  body->setLinearVelocity(btVector3(0,0,0));
  body->setAngularVelocity(btVector3(0,0,0));
  btVector3 translate = body->getCenterOfMassPosition();
  body->translate(-translate);
  body->translate(checkpoint);
  lives--;
}

void Physics::addGameObject(PhysicsBody* obj, int type, std::string name, btScalar x, btScalar y, btScalar z, btScalar angle, btScalar l, btScalar h, btScalar w, int property) {
  gameBodies.push_back(obj);
  int penguinCollidesWith = COL_WALL | COL_GROUND | COL_KILLBOX | COL_GOAL | COL_CHECKPOINT;
  int wallCollidesWith = COL_PENGUIN;
  int groundCollidesWith = COL_PENGUIN;
  int killboxCollidesWith = COL_PENGUIN;
  int goalCollidesWith = COL_PENGUIN;
  int checkpointCollidesWith = COL_PENGUIN;
  if (property & CHECKPOINT)
    dynamicWorld->addRigidBody(obj->getBody(), COL_CHECKPOINT, checkpointCollidesWith);
  if (property & KILLBOX)
    dynamicWorld->addRigidBody(obj->getBody(), COL_KILLBOX, killboxCollidesWith);
  if (property == NONE || property & INVISIBLE) {
    if (type == 0)
      dynamicWorld->addRigidBody(obj->getBody(), COL_PENGUIN, penguinCollidesWith);
    else if (type == 1)
      dynamicWorld->addRigidBody(obj->getBody(), COL_WALL, wallCollidesWith);
    else if (type == 2)
      dynamicWorld->addRigidBody(obj->getBody(), COL_GROUND, groundCollidesWith);
    else if (type == 3)
      dynamicWorld->addRigidBody(obj->getBody(), COL_GOAL, goalCollidesWith);
  }
  obj->getBody()->setUserPointer(gameBodies[gameBodies.size()-1]);
  graphics->addGameObject(type, name, x, y, z, angle, l, h, w, property);
}

void Physics::addPenguin(std::string name) {
  MotionState* motionState = new MotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,505,0)), graphics, name);
  btCollisionShape* shape = new btBoxShape(btVector3(30, 25, 25));
  btScalar mass = 1;
  btVector3 inertia(0,0,0);
  shape->calculateLocalInertia(mass,inertia);
  btRigidBody::btRigidBodyConstructionInfo info(mass,motionState,shape,inertia);
  btRigidBody* body = new btRigidBody(info);

  body->setFriction(1);
  body->setDamping(.4,.2);
  body->setRestitution(0);
  body->setActivationState(DISABLE_DEACTIVATION);
  body->setAngularFactor(btVector3(0, 1, 0));

  PhysicsBody* physicsBody = new PhysicsBody(body, motionState);
  addGameObject(physicsBody, 0, name, 0, 0, 0, 0, 0, 0, 0);
  checkpoint = btVector3(0, 505, 0);
}

void Physics::addWall(std::string name, btScalar x, btScalar y, btScalar z, btScalar angle, btScalar l, btScalar h, btScalar w, int property) {
  btCollisionShape* shape = new btBoxShape(btVector3(l,h,w));

  btScalar mass = 0;
  btVector3 intertia(0,0,0);
  shape->calculateLocalInertia(mass,intertia);

  MotionState* motionState = new MotionState(btTransform(btQuaternion(angle, 0, 0),btVector3(x, y, z)), graphics, name);

  btRigidBody::btRigidBodyConstructionInfo info(mass,motionState,shape,intertia);
  btRigidBody* body = new btRigidBody(info);
  body->setRestitution(.85);
  body->setLinearVelocity(btVector3(0,0,0));
  body->setFriction(1);
  body->setDamping(0,.2);
  body->setActivationState(DISABLE_DEACTIVATION);

  PhysicsBody* physicsBody = new PhysicsBody(body, motionState);
  addGameObject(physicsBody, 1, name, x, y, z, angle, l, h, w, property);
}

void Physics::addGoal(std::string name, btScalar x, btScalar y, btScalar z, btScalar angle) {
  btCollisionShape* shape = new btBoxShape(btVector3(10,10,10));

  btScalar mass = 0;
  btVector3 intertia(0,0,0);
  shape->calculateLocalInertia(mass,intertia);

  MotionState* motionState = new MotionState(btTransform(btQuaternion(angle, 0, 0),btVector3(x, y, z)), graphics, name);

  btRigidBody::btRigidBodyConstructionInfo info(mass,motionState,shape,intertia);
  btRigidBody* body = new btRigidBody(info);
  body->setRestitution(.85);
  body->setLinearVelocity(btVector3(0,0,0));
  body->setFriction(1);
  body->setDamping(0,.2);
  body->setActivationState(DISABLE_DEACTIVATION);

  PhysicsBody* physicsBody = new PhysicsBody(body, motionState);
  addGameObject(physicsBody, 3, name, x, y, z, angle, 50, 50, 50);
}

void Physics::addGround(std::string name, btScalar x, btScalar y, btScalar z, btScalar angle, btScalar l, btScalar h, btScalar w, int property) {
  btCollisionShape* shape = new btBoxShape(btVector3(l,h,w));

  btScalar mass(0.);
  btVector3 inertia(0,0,0);
  shape->calculateLocalInertia(mass,inertia);

  MotionState* motionState = new MotionState(btTransform(btQuaternion(angle, 0, 0),btVector3(x, y, z)), graphics, name);

  btRigidBody::btRigidBodyConstructionInfo info(mass, motionState, shape, inertia);
  btRigidBody* body = new btRigidBody(info);
  body->setActivationState(DISABLE_DEACTIVATION);

  PhysicsBody* physicsBody = new PhysicsBody(body, motionState);
  addGameObject(physicsBody, 2, name, x, y, z, 0, l, h, w, property);
}

void Physics::removeStage(void) {
  int x = 1;
  while (gameBodies.size() > x) {
    PhysicsBody* obj = gameBodies.at(x);
    dynamicWorld->removeRigidBody(obj->getBody());
    gameBodies.remove(obj);
    delete obj;
    graphics->removeObject(x);
  }
}

void Physics::translate(int index, btScalar x, btScalar y, btScalar z) {
  btRigidBody* body = gameBodies.at(index)->getBody();
  body->translate(btVector3(x, y, z));
}

void Physics::rotate(int index, btScalar angle) {
  btRigidBody* body = gameBodies.at(index)->getBody();
  btQuaternion quat;
  quat.setEuler(angle,0,0);
  btTransform transform = body->getCenterOfMassTransform();
  transform.setRotation(transform.getRotation() * quat);
  body->setCenterOfMassTransform(transform);
}

void Physics::applyForce(int index, btScalar x, btScalar y, btScalar z) {
  btRigidBody* body = gameBodies.at(index)->getBody();
  body->applyCentralForce(btVector3(x, y, z));
  graphics->playSound(0);
}

bool Physics::gameOver() {
  return lives < 0;
}

#ifndef __PhysicsBody_h_
#define __PhysicsBody_h_

#include "MotionState.h"

class MotionState;

class PhysicsBody {
  public:
    PhysicsBody(btRigidBody* rigidBody, MotionState* ms);
    virtual ~PhysicsBody(void);
    btRigidBody* getBody();
    btRigidBody* setBody(btRigidBody* rigidBody);
    btRigidBody* setMotion(MotionState* motionState);
    
  private:
    btRigidBody* body;
    MotionState* ms;
};

#endif // #ifndef __PhysicsBody_h_
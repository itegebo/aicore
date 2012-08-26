/*
 * The kinematic movement demo.
 *
 * Part of the Artificial Intelligence for Games system.
 *
 * Copyright (c) Ian Millington 2003-2006. All Rights Reserved.
 *
 * This software is distributed under licence. Use of this software
 * implies agreement with all terms and conditions of the accompanying
 * software licence.
 */

#include <gl/glut.h>

#include "../common/gl/app.h"
#include <aicore/kinematic.h>
#include <aicore/timing.h>

/**
 * The main demo class.
 */
class KinematicMovementDemo : public Application
{
  static const int num_boids = 2;

  // This is the size of the world in both directions from 0 (i.e. from
  // -WORLD_SIZE to +WORLD_SIZE)
  static const int world_size = 50;

  // This is the gap between grid lines
  static const int grid_size = 5;

  /** Holds the location of the two agents. */
  aicore::Location location[num_boids];

  /** Holds the steering behaviours. */
  aicore::KinematicSeek seek[num_boids];
  aicore::KinematicFlee flee[num_boids];
  aicore::KinematicArrive arrive[num_boids];
  aicore::KinematicWander wander[num_boids];

  /** Holds the currently used behaviours. */
  aicore::KinematicMovement* current[2];
  const char* status[2];

public:
  KinematicMovementDemo();
  virtual ~KinematicMovementDemo();

  virtual bool key(unsigned char key);
  virtual void update();
  virtual void display();
  virtual const char* getTitle();
  virtual void getHelpText(const char*** lines, unsigned *lineCount);
  virtual unsigned getStatusCount();
  virtual const char* getStatusText(unsigned slot);

  static void trim_world(aicore::real& var);
};


KinematicMovementDemo::KinematicMovementDemo()
  :
  Application()
{

  location[0].position.x = 10.0f;
  location[0].position.z = 10.0f;
  location[0].orientation = 2.1f;

  location[1].position.x = -10.0f;
  location[1].position.z = -20.0f;
  location[1].orientation = 0.75f;

  for (unsigned i = 0; i < num_boids; i++) {
    seek[i].character = &location[i];
    seek[i].target = &location[1-i].position;
    seek[i].maxSpeed = (aicore::real)10.0;

    flee[i].character = &location[i];
    flee[i].target = &location[1-i].position;
    flee[i].maxSpeed = (aicore::real)10.0;

    arrive[i].character = &location[i];
    arrive[i].target = &location[1-i].position;
    arrive[i].maxSpeed = (aicore::real)10.0;
    arrive[i].timeToTarget = (aicore::real)2.0;
    arrive[i].radius = (aicore::real)1.0;

    wander[i].character = &location[i];
    wander[i].maxSpeed = (aicore::real)10.0;
    wander[i].maxRotation = (aicore::real)8.0 * aicore::M_PI;
  }

  // Set the current behaviours
  //current = new aicore::KinematicMovement*[2];
  //status = new const char*[2];

  current[0] = &arrive[0];
  status[0] = "Arrive";
  current[1] = &flee[1];
  status[1] = "Flee";
}

KinematicMovementDemo::~KinematicMovementDemo() {}

static const char *defaultHelp[] = {
  "AI4G: Kinematic Movement Demo",
  "H - Toggle help.",
  "",
  "Red character:",
  "Q - Stationary",
  "W - Seek",
  "E - Flee",
  "R - Arrive",
  "T - Wander",
  "",
  "Green character:",
  "A - Stationary",
  "S - Seek",
  "D - Flee",
  "F - Arrive",
  "G - Wander",
};

void KinematicMovementDemo::getHelpText(const char*** lines,
                                        unsigned *lineCount)
{
  *lines = defaultHelp;
  *lineCount = sizeof(defaultHelp) / sizeof(const char*);
}

const char* KinematicMovementDemo::getTitle()
{
  return defaultHelp[0];
}

bool KinematicMovementDemo::key(unsigned char key)
{
  if (Application::key(key)) return true;

  switch(key) {
  case 'Q': case 'q':
    current[0] = NULL;
    status[0] = "Stationary";
    return true;
  case 'W': case 'w':
    current[0] = &seek[0];
    status[0] = "Seek";
    return true;
  case 'E': case 'e':
    current[0] = &flee[0];
    status[0] = "Flee";
    return true;
  case 'R': case 'r':
    current[0] = &arrive[0];
    status[0] = "Arrive";
    return true;
  case 'T': case 't':
    current[0] = &wander[0];
    status[0] = "Wander";
    return true;

  case 'A': case 'a':
    current[1] = NULL;
    status[1] = "Stationary";
    return true;
  case 'S': case 's':
    current[1] = &seek[1];
    status[1] = "Seek";
    return true;
  case 'D': case 'd':
    current[1] = &flee[1];
    status[1] = "Flee";
    return true;
  case 'F': case 'f':
    current[1] = &arrive[1];
    status[1] = "Arrive";
    return true;
  case 'G': case 'g':
    current[1] = &wander[1];
    status[1] = "Wander";
    return true;
  }

  return false;
}

void KinematicMovementDemo::display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  gluLookAt(-53.0f, 53.0f, 0.0f,
            0.0f, -30.0f, 0.0f,
            0.0f, 1.0f, 0.0f);

  // Draw the characters.
  glColor3f(0.6f, 0.0f, 0.0f);
  renderAgent(location[0]);
  glColor3f(0.0f, 0.6f, 0.0f);
  renderAgent(location[1]);

  // Draw some scale lines
  glColor3f(0.8, 0.8, 0.8);
  glBegin(GL_LINES);
  for (int i = -world_size; i <= world_size; i += grid_size) {

    glVertex3i(-world_size, -1, i);
    glVertex3i(world_size, -1, i);

    glVertex3i(i, -1, world_size);
    glVertex3i(i, -1, -world_size);
  }
  glEnd();

  // Draw the help (the method decides if it should be displayed)
  displayHelp();
}

void KinematicMovementDemo::trim_world(aicore::real& var) {
  if (var < -world_size) var = world_size;
  if (var > world_size) var = -world_size;
}

/*

A body has a velocity and position
  also, a max speeds, both linear and angular

A body can be given steering behaviors if it satisfies the
requirements of those behaviors

A process of calculating changes to some of these values is called
"Newton-Euler-1 integration update".

A universe has time that measures the movement of bodies.  One
"observes" the universe at a point in time and thus, a particular
arrangement of bodies.  An observation can be visualized, or rendered
into an image.

A view (visualized observation) is seen by a viewer.  A viewer has a
perspective.

---

So, the universe can be a list of bodies - some bodies might be fixed
and thus represent surfaces, obstacles, etc.

I don't know that there's a need for multiple "observers".  Although,
you may have an AI thread observing the universe as well as a human
viewer who wants a visualization.  This generalizes nicely to "shared
observations" between the AI and the human.

*/

void KinematicMovementDemo::update()
{
  float duration = aicore::TimingData::get().lastFrameDuration * 0.001f;
  aicore::SteeringOutput steer;

  for (unsigned i = 0; i < num_boids; i++) {

    // Update the steering if we have a movement algorithm
    if (current[i]) current[i]->getSteering(&steer);
    else steer.clear();

    location[i].integrate(steer, duration);

    // Normalise the orientation, unless we're wandering (in which
    // case this will destroy the wander behaviour).
    if (current[i] != &wander[i]) {
      location[i].setOrientationFromVelocity(steer.linear);
    }

    trim_world(location[i].position.x);
    trim_world(location[i].position.z);
  }

  glutPostRedisplay();
}

unsigned KinematicMovementDemo::getStatusCount()
{
  return 2;
}

const char* KinematicMovementDemo::getStatusText(unsigned slot)
{
  // Set the color
  switch(slot)
    {
    case 0:
      glColor3f(0.6f, 0.0f, 0.0f);
      break;
    case 1:
      glColor3f(0.0f, 0.6f, 0.0f);
      break;
    }

  // Return the text
  return status[slot];
}





Application* getApplication()
{
  return new KinematicMovementDemo();
}

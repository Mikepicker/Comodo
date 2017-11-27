#include <irrlicht.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <map>

using namespace irr;

#include "input_receiver.cpp"

#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif

// irrlicht
IrrlichtDevice *device;
scene::ISceneManager* smgr;
scene::ISceneCollisionManager* collMan;
InputReceiver inputReceiver;

// constants
const s32 SCREEN_WIDTH = 640;
const s32 SCREEN_HEIGHT = 480;
const s32 MAP_WIDTH = 30;
const s32 MAP_HEIGHT = 10;
const s32 MAX_ENEMIES = 10;
const s32 GRAVITY = 10;
const s32 JUMP_V = 5;
const s32 RAY_LENGTH = 10000;
const s32 MISSILE_SPEED = 800;
const s32 LOCK_TIME = 1500;
u32 then;

// data
video::ITexture* currentAim;
video::ITexture* aimWhite;
video::ITexture* aimYellow;
video::ITexture* aimRed;
scene::IParticleEmitter* flaresEm;

// entities
scene::ISceneNode* sun;

struct Balloon;
struct Missile {
    scene::ISceneNode* node;
    scene::ISceneNodeAnimatorCollisionResponse* collider;
    Balloon* target;
    bool alive;
} missile;

struct Balloon {
    scene::ISceneNode* node;
    scene::ISceneNodeAnimatorCollisionResponse* collider;
    scene::IParticleSystemSceneNode* flaresPs;
    core::vector3df v;
    Missile missile;
    bool jump;
    Balloon* target;
    bool enemyLocked;
    u32 enemyLockedSince;
    bool flaresOn;
    u32 flaresStart;
    bool alive;
};

struct Player {
    Balloon balloon;    
} player;

struct Enemy {
    std::string name;
    Balloon balloon;
    std::string state;
    core::vector3df searchPoint;
};

// enemies
Enemy enemies[MAX_ENEMIES];

// collision masks
enum {
    id_map = 0,
    id_enemy = 1 << 0,
    id_player = 1 << 1
};

#include "utils.cpp"
#include "system_missile.cpp"
#include "system_ai.cpp"

// Update
void update() {
    
    const u32 now = device->getTimer()->getTime();
    const f32 dt = (f32)(now - then) / 1000.f; // Time in seconds
    then = now;

    // apply gravity
    core::vector3df v = player.balloon.v;
    v.Y -= GRAVITY * dt;

    if (player.balloon.collider->collisionOccurred() &&
        v.Y < 0 && 
        player.balloon.collider->getCollisionPoint().Y < player.balloon.node->getPosition().Y) {
        v.Y = 0;
    }

    // apply velocity
    core::vector3df playerPos = player.balloon.node->getPosition();
    playerPos.Y += v.Y;
    player.balloon.node->setPosition(playerPos);

    // jump
    if (inputReceiver.IsKeyDown(irr::KEY_SPACE)) {
        if (!player.balloon.jump) {
            v.Y = v.Y < 0 ? JUMP_V : v.Y + JUMP_V;
            player.balloon.jump = true;
        }
    } else {
        player.balloon.jump = false;
    }

    // set new velocity
    player.balloon.v = v;

    // get aimed target
    scene::ICameraSceneNode* camera = (scene::ICameraSceneNode*)player.balloon.node;
    
    core::vector3df hitPoint;
    core::triangle3df triangle;
    core::line3d<f32> ray;
    ray.start = camera->getPosition();
    ray.end = ray.start + (camera->getTarget() - ray.start).normalize() * RAY_LENGTH;
    scene::ISceneNode* selectedSceneNode = collMan->getSceneNodeAndCollisionPointFromRay(ray, hitPoint, triangle);

    if (selectedSceneNode && (selectedSceneNode->getID() & id_enemy) == id_enemy) {
      
        // change aim
        currentAim = aimYellow;

        // set enemyLocked
        if (!player.balloon.enemyLocked) {
            player.balloon.enemyLocked = true;
            player.balloon.enemyLockedSince = now;
        } else if (now - player.balloon.enemyLockedSince > LOCK_TIME) {
            currentAim = aimRed;
            
            // Shoot missile
            if (inputReceiver.GetMouseState().LeftButtonDown) {
                Enemy* enemy = findEnemyByName(selectedSceneNode->getName());
                shootMissile(&player.balloon.missile, &(enemy->balloon), player.balloon.node->getPosition());
            }
        }
        
    } else {
        player.balloon.enemyLocked = false;
        currentAim = aimWhite;
    }

    // Missiles
    systemMissile(&player.balloon.missile, dt);

    // flares
    if (player.balloon.flaresOn) {
        if (now - player.balloon.flaresStart > 500) {
            player.balloon.flaresPs->setEmitter(0);
            player.balloon.flaresOn = false;
        }
    } else {
        if (inputReceiver.GetMouseState().RightButtonDown) {
            player.balloon.flaresPs->setEmitter(flaresEm);
            player.balloon.flaresOn = true;
            player.balloon.flaresStart = now;
        }
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        systemAI(&enemies[i], dt);
        systemMissile(&enemies[i].balloon.missile, dt);
    }
    
    // sun
    /*core::vector3df sunPos = sun->getPosition();
    sunPos.X += 100 * dt;
    sun->setPosition(sunPos);*/
}

int main() {

    // seed random
    srand(time(NULL));

    SIrrlichtCreationParameters params;
    params.AntiAlias = 2;
    params.Bits = 16;
    params.WindowSize = core::dimension2d<u32>(SCREEN_WIDTH, SCREEN_HEIGHT);
    params.DriverType = video::EDT_OPENGL;
    params.Stencilbuffer = true;
    params.EventReceiver = &inputReceiver;

	device = createDeviceEx(params);

	if (device == 0)
		return 1; // could not create selected driver.

	/*
	Get a pos32er to the video driver and the SceneManager so that
	we do not always have to call irr::IrrlichtDevice::getVideoDriver() and
	irr::IrrlichtDevice::getSceneManager().
	*/
	video::IVideoDriver* driver = device->getVideoDriver();
	smgr = device->getSceneManager();
    collMan = smgr->getSceneCollisionManager();

    // set ambient light
    smgr->setAmbientLight(video::SColorf(1.0f,1.0f,1.0f));

    // load hexagons
    scene::IAnimatedMesh* mesh = smgr->getMesh("assets/hexagon_green.obj");
    smgr->getMeshManipulator()->setVertexColors(mesh, video::SColor(1,100,200,40));
    scene::IMeshSceneNode* node = 0;
    scene::IMetaTriangleSelector* metaSelector = smgr->createMetaTriangleSelector();
    for (s32 i = 0; i < MAP_WIDTH; i++) {
        for (s32 j = 0; j < MAP_HEIGHT; j++) {
            if (mesh) {
                node = smgr->addOctreeSceneNode(mesh->getMesh(0), 0, id_map, 1024);
                s32 offX = i % 2 == 1 ? 300 : 0;
                s32 offZ = sqrt(3)*200/2;
                s32 rand = randInRange(-500, 500);
                node->setPosition(core::vector3df(600*j + offX,-4000 + rand,offZ*i));
                node->addShadowVolumeSceneNode();
                //node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
                node->setMaterialFlag(video::EMF_LIGHTING, true);

                // triangle selector
                scene::ITriangleSelector* selector = smgr->createOctreeTriangleSelector(mesh, node);
                node->setTriangleSelector(selector);
                metaSelector->addTriangleSelector(selector);
                selector->drop();
            }
        }
    }

    // set shadows
    smgr->setShadowColor(video::SColor(150,0,0,0));

    // aim
    aimWhite = driver->getTexture("assets/aim_white.png");
    aimYellow = driver->getTexture("assets/aim_yellow.png");
    aimRed = driver->getTexture("assets/aim_red.png");
    currentAim = aimWhite;

    // light
    float i = 0.7f;
    sun = smgr->addLightSceneNode(0, core::vector3df(0,4000,0),
            video::SColorf(i, i, i, i), 10000.0f);
    scene::ISceneNodeAnimator* anim = 0;
    /*anim = smgr->createFlyCircleAnimator(core::vector3df(0,150,0), 250.0f);
    light->addAnimator(anim);*/
    //anim->drop();

    // attach billboard to light
    scene::ISceneNode* billboard = smgr->addBillboardSceneNode(sun, core::dimension2d<f32>(50, 50));
    billboard->setMaterialFlag(video::EMF_LIGHTING, false);
    billboard->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
    billboard->setMaterialTexture(0, driver->getTexture("assets/particlewhite.bmp"));

    // balloon mesh
    scene::IAnimatedMesh* balloonMesh = smgr->getMesh("assets/balloon.obj");

    // player
    SKeyMap keyMap[8];
    keyMap[0].Action = EKA_MOVE_FORWARD;
    keyMap[0].KeyCode = KEY_UP;
    keyMap[1].Action = EKA_MOVE_FORWARD;
    keyMap[1].KeyCode = KEY_KEY_W;

    keyMap[2].Action = EKA_MOVE_BACKWARD;
    keyMap[2].KeyCode = KEY_DOWN;
    keyMap[3].Action = EKA_MOVE_BACKWARD;
    keyMap[3].KeyCode = KEY_KEY_S;

    keyMap[4].Action = EKA_STRAFE_LEFT;
    keyMap[4].KeyCode = KEY_LEFT;
    keyMap[5].Action = EKA_STRAFE_LEFT;
    keyMap[5].KeyCode = KEY_KEY_A;

    keyMap[6].Action = EKA_STRAFE_RIGHT;
    keyMap[6].KeyCode = KEY_RIGHT;
    keyMap[7].Action = EKA_STRAFE_RIGHT;
    keyMap[7].KeyCode = KEY_KEY_D;

    player.balloon.node = smgr->addCameraSceneNodeFPS(0, 100, 1, -1, keyMap, 8, false);
    player.balloon.node->setPosition(core::vector3df(0, 1000, 0));
    ((scene::ICameraSceneNode*)(player.balloon.node))->setFarValue(30000.0f);
    player.balloon.v = core::vector3df(0, 0, 0);

    // animator (collisions)
    player.balloon.collider = smgr->createCollisionResponseAnimator(
            metaSelector, player.balloon.node, core::vector3df(30, 50, 30),
            core::vector3df(0, 0, 0), core::vector3df(0, 30, 0));

    player.balloon.node->addAnimator(player.balloon.collider);
    metaSelector->drop();


    scene::IAnimatedMeshSceneNode* playerMesh = smgr->addAnimatedMeshSceneNode(balloonMesh, 0, id_player);
    playerMesh->setPosition(core::vector3df(0, 0, 300));
    scene::ITriangleSelector* selector = smgr->createTriangleSelectorFromBoundingBox(playerMesh);
    playerMesh->setTriangleSelector(selector);
    player.balloon.node->addChild(playerMesh);

    
    // enemy
    Enemy enemy;
    smgr->getMeshManipulator()->setVertexColors(balloonMesh, video::SColor(1,0,0,255));
    enemy.balloon.node = smgr->addAnimatedMeshSceneNode(balloonMesh, 0, id_enemy);
    enemy.balloon.node->setPosition(core::vector3df(600, 800, 1000));
    selector = smgr->createTriangleSelectorFromBoundingBox(enemy.balloon.node);
    enemy.balloon.node->setTriangleSelector(selector);
    selector->drop();

    enemy.balloon.node->setName("enemy_1");
    enemy.name = "enemy_1";
    enemy.state = "state_idle";


    balloonMesh->drop();

    // player missile
    player.balloon.missile.node = smgr->addSphereSceneNode(10.0f);
    player.balloon.missile.alive = false;
    player.balloon.missile.node->setVisible(false);
    player.balloon.missile.node->setPosition(player.balloon.node->getPosition());
    player.balloon.missile.collider = smgr->createCollisionResponseAnimator(
            enemy.balloon.node->getTriangleSelector(), player.balloon.missile.node, core::vector3df(10,10,10), core::vector3df(0,0,0));

    player.balloon.missile.node->addAnimator(player.balloon.missile.collider);

    // enemy missile
    enemy.balloon.missile.node = smgr->addSphereSceneNode(10.0f);
    enemy.balloon.missile.alive = false;
    enemy.balloon.missile.node->setVisible(false);
    enemy.balloon.missile.node->setPosition(enemy.balloon.node->getPosition());
    enemy.balloon.missile.collider = smgr->createCollisionResponseAnimator(
            player.balloon.node->getTriangleSelector(), enemy.balloon.missile.node, core::vector3df(10,10,10), core::vector3df(0,0,0));

    enemy.balloon.missile.node->addAnimator(enemy.balloon.missile.collider);

    enemies[0] = enemy;

    // particle system
    scene::IParticleSystemSceneNode* ps =
                smgr->addParticleSystemSceneNode(false);

    flaresEm = ps->createBoxEmitter(
        core::aabbox3d<f32>(-7,0,-7,7,1,7), // emitter size
        core::vector3df(0.0f,-0.5f,0.0f),   // initial direction
        80,100,                             // emit rate
        video::SColor(0,255,255,255),       // darkest color
        video::SColor(0,255,255,255),       // brightest color
        800,2000,0,                         // min and max age, angle
        core::dimension2df(10.f,10.f),         // min size
        core::dimension2df(20.f,20.f));        // max size  


    ps->setPosition(core::vector3df(0,-15,0));
    ps->setScale(core::vector3df(2,2,2));
    ps->setMaterialFlag(video::EMF_LIGHTING, false);
    ps->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
    ps->setMaterialTexture(0, driver->getTexture("assets/fire.bmp"));
    ps->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
    player.balloon.node->addChild(ps);
    player.balloon.flaresPs = ps;

	/*
	The mouse cursor needs not be visible, so we hide it via the
	irr::IrrlichtDevice::ICursorControl.
	*/
	device->getCursorControl()->setVisible(false);

    /*driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);

    // Skybox and skydome
    scene::ISceneNode* skybox = smgr->addSkyBoxSceneNode(
            driver->getTexture("assets/irrlicht2_up.jpg"),
            driver->getTexture("assets/irrlicht2_dn.jpg"),
            driver->getTexture("assets/irrlicht2_lf.jpg"),
            driver->getTexture("assets/irrlicht2_rt.jpg"),
            driver->getTexture("assets/irrlicht2_ft.jpg"),
            driver->getTexture("assets/irrlicht2_bk.jpg"));
    
    scene::ISceneNode* skydome = smgr->addSkyDomeSceneNode(driver->getTexture("assets/skydome.jpg"), 16, 8, 0.95f, 2.0f);

    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);*/

	/*
	We have done everything, so lets draw it. We also write the current
	frames per second and the primitives drawn s32o the caption of the
	window. The test for irr::IrrlichtDevice::isWindowActive() is optional,
	but prevents the engine to grab the mouse cursor after task switching
	when other programs are active. The call to
	irr::IrrlichtDevice::yield() will avoid the busy loop to eat up all CPU
	cycles when the window is not active.
	*/
	s32 lastFPS = -1;

    // calculate delta
    then = device->getTimer()->getTime();

	while(device->run()) {
		if (device->isWindowActive()) {
			driver->beginScene(true, true, video::SColor(255,150,200,249));
            update();
			smgr->drawAll();

            // draw aim
            driver->draw2DImage(currentAim, core::position2d<s32>((SCREEN_WIDTH/2) - 4, (SCREEN_HEIGHT/2) - 4), core::rect<s32>(0, 0, 8, 8),
                    0, video::SColor(255, 255, 255, 255), true);
			driver->endScene();

			s32 fps = driver->getFPS();

			if (lastFPS != fps)
			{
				core::stringw str = L"Comodo [";
				str += driver->getName();
				str += "] FPS:";
				str += fps;

				device->setWindowCaption(str.c_str());
				lastFPS = fps;
			}
		}
		else
			device->yield();
	}

	/*
	In the end, delete the Irrlicht device.
	*/
	device->drop();
	return 0;
}

/*
That's it. Compile and play around with the program.
**/

#include <irrlicht.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>

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
video::IVideoDriver* driver;
gui::IGUIFont* font;

// constants
const s32 SCREEN_WIDTH = 640;
const s32 SCREEN_HEIGHT = 480;
const s32 STAGE_WIDTH = 5500;
const s32 STAGE_DEPTH = 5000;
const s32 MAP_WIDTH = 30;
const s32 MAP_HEIGHT = 10;
const s32 MAX_ENEMIES = 100;
const s32 GRAVITY = 10;
const s32 ENEMY_SPEED = 200;
const s32 JUMP_V = 5;
const s32 RAY_LENGTH = 10000;
const s32 MISSILE_SPEED = 800;
const s32 MISSILE_TIME = 5000;
const s32 LOCK_TIME = 1500;
const s32 FLARES_TIME = 500;
const s32 FLARES_COOLDOWN = 5000;
const s32 DEATHSPHERE_SPEED = 10000;
const f32 DEATHSPHERE_RADIUS = 10000.0f;

u32 then;

// data
video::ITexture* currentAim;
video::ITexture* aimWhite;
video::ITexture* aimYellow;
video::ITexture* aimRed;

scene::IMetaTriangleSelector* metaSelector;

// entities
scene::ISceneNode* sun;

struct Balloon;
struct Missile {
    scene::ISceneNode* node;
    scene::ISceneNodeAnimatorCollisionResponse* collider;
    scene::IParticleSystemSceneNode* missilePs;
    scene::IParticleEmitter* missileEm;
    core::vector3df v;
    Balloon* target;
    bool alive;
    u32 shootTime;
} missile;

struct Crate {
    scene::ISceneNode* node;
} crate;

struct Balloon {
    scene::ISceneNode* node;
    scene::ISceneNodeAnimatorCollisionResponse* collider;
    scene::IParticleSystemSceneNode* flaresPs;
    scene::IParticleEmitter* flaresEm;
    scene::IParticleSystemSceneNode* explosionPs;
    scene::IParticleEmitter* explosionEm;
    core::vector3df v;
    Missile missile;
    bool jump;
    Balloon* target;
    bool enemyLocked;
    u32 enemyLockedSince;
    Balloon* lockedBy;
    bool flaresOn;
    u32 flaresStart;
    f32 timeToFlares;
    bool explosionOn;
    u32 explosionStart;
    bool isLocked;
    bool alive;
    s32 scale;
};

struct Player {
    Balloon balloon;    
} player;

struct Enemy {
    core::stringc name;
    Balloon balloon;
    std::string state;
    core::vector3df searchPoint;
    scene::ISceneNode* sensor;
    scene::ISceneNodeAnimatorCollisionResponse* sensorCollider;
};

struct DeathSphere {
    scene::ISceneNode* node;
    scene::ISceneNodeAnimatorCollisionResponse* collider;
} deathSphere;

// enemies
Enemy enemies[MAX_ENEMIES];
s32 enemiesAlive = 0;

// collision masks
enum {
    id_map = 0,
    id_enemy = 1 << 0,
    id_player = 1 << 1,
    id_death = 1 << 2
};

#include "utils.cpp"
#include "system_missile.cpp"
#include "system_player.cpp"
#include "system_enemy.cpp"
#include "system_balloon.cpp"
#include "system_crate.cpp"

// Update
void update() {
    
    const u32 now = device->getTimer()->getTime();
    const f32 dt = (f32)(now - then) / 1000.f; // Time in seconds
    then = now;

    // reset game
    if (inputReceiver.IsKeyDown(irr::KEY_KEY_R) && !player.balloon.alive) {
        initPlayerData();
        deathSphere.node->setPosition(core::vector3df(-100000, 0, 1000));
        for (s32 i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].balloon.alive) {
                enemies[i].balloon.alive = false;
                enemies[i].balloon.node->setVisible(false);
                enemies[i].balloon.missile.alive = false;
                enemies[i].balloon.missile.node->setVisible(false);
            }
        }
        enemiesAlive = 0;
        
        return;
    }

    // Player
    systemPlayer(dt);

    // Missiles
    systemMissile(&player.balloon.missile, dt);

    // Balloons
    systemBalloon(&(player.balloon), dt);

    systemAI(dt);

    for (s32 i = 0; i < MAX_ENEMIES; i++) {
        systemMissile(&enemies[i].balloon.missile, dt);
        systemBalloon(&enemies[i].balloon, dt);
    }

    // crate
    systemCrate(dt);

    // death sphere
    core::vector3df pos = deathSphere.node->getPosition();
    pos.X += DEATHSPHERE_SPEED * dt;
    deathSphere.node->setPosition(pos);

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
	driver = device->getVideoDriver();
	smgr = device->getSceneManager();
    collMan = smgr->getSceneCollisionManager();

    //smgr->getParameters()->setAttribute(scene::COLLADA_CREATE_SCENE_INSTANCES, true);

    // font
    font = device->getGUIEnvironment()->getFont("assets/font.bmp");

    // set ambient light
    smgr->setAmbientLight(video::SColorf(1.0f,1.0f,1.0f));

    // load hexagons
    scene::IAnimatedMesh* mesh = smgr->getMesh("assets/hexagon_green.obj");
    smgr->getMeshManipulator()->setVertexColors(mesh, video::SColor(1,100,200,40));
    scene::IMeshSceneNode* node = 0;
    metaSelector = smgr->createMetaTriangleSelector();
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

    // death spere
    deathSphere.node = smgr->addSphereSceneNode(DEATHSPHERE_RADIUS, 64, 0, id_death);
    deathSphere.node->setPosition(core::vector3df(-100000, 0, 1000));
    /*deathSphere.collider = smgr->createCollisionResponseAnimator(
                player.balloon.node->getTriangleSelector(), deathSphere.node, core::vector3df(100000), core::vector3df(0,0,0));
    deathSphere.node->addAnimator(deathSphere.collider);*/


    // create player
    createPlayer();

    // player missile
    createMissile(&player.balloon);
   
    // create crate
    createCrate();

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

            if (!player.balloon.alive) {
                font->draw(L"Game Over - Press R to restart", core::rect<s32>(SCREEN_WIDTH/2 - 80, 10, 300, 10), video::SColor(255, 255, 255, 255));
            } else if (enemiesAlive == 100) {
                font->draw("You Won!", core::rect<s32>(SCREEN_WIDTH/2 - 20, 10, 300, 10), video::SColor(255, 255, 255, 255));
            } else {
                core::stringw text = "Balloons ";
                text += enemiesAlive;
                text += "/100";
                font->draw(text, core::rect<s32>(SCREEN_WIDTH/2 - 30, 10, 300, 10), video::SColor(255, 255, 255, 255));
            }

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

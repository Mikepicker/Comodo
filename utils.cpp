s32 randInRange(s32 min, s32 max) {
    return rand() % (max + 1 - min) + min;
}

core::vector3df randPosition() {
    return core::vector3df(randInRange(0, STAGE_WIDTH), 800, randInRange(0, STAGE_DEPTH));
}

Enemy* findEnemyByName(core::stringw name) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].name == name) {
            return &enemies[i];
        }
    }
}

video::SColor randColor() {
    return video::SColor(1, randInRange(0, 255), randInRange(0, 255), randInRange(0, 255));
}

void attachExplosionPs(Balloon* balloon) {
    scene::IParticleSystemSceneNode* ps =
                smgr->addParticleSystemSceneNode(false);

    scene::IParticleEmitter* explEm = 
        ps->createBoxEmitter(
            core::aabbox3d<f32>(-7,0,-7,7,1,7), // emitter size
            core::vector3df(0.0f,-0.5f,0.0f),   // initial direction
            1000,2000,                             // emit rate
            video::SColor(0,255,0,0),           // darkest color
            video::SColor(0,255,255,255),       // brightest color
            800,2000,180,                       // min and max age, angle
            core::dimension2df(10.f,10.f),      // min size
            core::dimension2df(20.f,20.f));     // max size  

    ps->setPosition(core::vector3df(0,0,0));
    ps->setScale(core::vector3df(2,2,2));
    ps->setMaterialFlag(video::EMF_LIGHTING, false);
    ps->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
    ps->setMaterialTexture(0, driver->getTexture("assets/fire.bmp"));
    ps->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);

    balloon->explosionEm = explEm;
    balloon->explosionPs = ps;
}

void explode(Balloon* balloon) {
    const u32 now = device->getTimer()->getTime();
    balloon->explosionStart = now;
    balloon->explosionOn = true;
    balloon->explosionPs->setPosition(balloon->node->getPosition());
    balloon->explosionPs->setEmitter(balloon->explosionEm);
    balloon->alive = false;
}

void attachMissilePs(Missile* missile) {
    scene::IParticleSystemSceneNode* ps =
                smgr->addParticleSystemSceneNode(false);

    scene::IParticleEmitter* em = 
        ps->createBoxEmitter(
            core::aabbox3d<f32>(-7,0,-7,7,1,7), // emitter size
            core::vector3df(0.0f,0.0f,0.0f),   // initial direction
            80,100,                             // emit rate
            video::SColor(0,255,0,0),           // darkest color
            video::SColor(0,255,255,255),       // brightest color
            800,2000,0,                       // min and max age, angle
            core::dimension2df(10.f,10.f),      // min size
            core::dimension2df(20.f,20.f));     // max size  

    ps->setPosition(core::vector3df(0,0,0));
    ps->setScale(core::vector3df(2,2,2));
    ps->setMaterialFlag(video::EMF_LIGHTING, false);
    ps->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
    ps->setMaterialTexture(0, driver->getTexture("assets/fire.bmp"));
    ps->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
    ps->setEmitter(em);

    missile->node->addChild(ps);
    missile->missileEm = em;
    missile->missilePs = ps;
}

void attachFlaresPs(Balloon* balloon) {

    // flares
    scene::IParticleSystemSceneNode* ps =
                smgr->addParticleSystemSceneNode(false);

    scene::IParticleEmitter* flaresEm = ps->createBoxEmitter(
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
    balloon->node->addChild(ps);
    balloon->flaresPs = ps;
    balloon->flaresEm = flaresEm;
}

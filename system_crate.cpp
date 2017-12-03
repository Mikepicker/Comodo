void changePosition() {
    crate.node->setPosition(randPosition());
    crate.node->setMaterialTexture(0, driver->getTexture("assets/crate.png"));
    crate.node->setMaterialType(video::EMT_SOLID);
}

void createCrate() {
    
    crate.node = smgr->addCubeSceneNode(100.0f);
    changePosition();
}

void systemCrate(f32 dt) {

    const u32 now = device->getTimer()->getTime();
    crate.node->setRotation(core::vector3df(0, now/20, 0));

    // player picks crate up
    f32 dist = (crate.node->getPosition() - player.balloon.node->getPosition()).getLength();
    if (dist < 100) {
        changePosition();
        createEnemy(); 
    }
}

void changePosition() {
    crate.node->setPosition(randPosition());
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

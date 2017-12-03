void createMissile(Balloon* balloon) {
    balloon->missile.node = smgr->addSphereSceneNode(4.0f);
    balloon->missile.alive = false;
    balloon->missile.node->setVisible(false);
    balloon->missile.node->setPosition(balloon->node->getPosition());

    attachMissilePs(&balloon->missile);

    balloon->missile.collider = smgr->createCollisionResponseAnimator(
            metaSelector, balloon->missile.node, core::vector3df(4), core::vector3df(0,0,0));
    balloon->missile.node->addAnimator(balloon->missile.collider);
} 

void shootMissile(Missile* missile, Balloon* target, core::vector3df startPos) {
    if (!missile->alive) {
        missile->node->setPosition(startPos);
        missile->target = target;
        missile->alive = true; 
        missile->node->setVisible(true);
        
        const u32 now = device->getTimer()->getTime();
        missile->shootTime = now;

        // update target
        core::vector3df missileDist = startPos - target->node->getPosition();
        f32 timeToFlares = ((missileDist.getLength()/MISSILE_SPEED)/4)*1000;
        target->timeToFlares = now + timeToFlares;
    }
}

void systemMissile(Missile* missile, f32 dt) {
    if (missile->alive) {

        const u32 now = device->getTimer()->getTime();
        if (now - missile->shootTime >= MISSILE_TIME) {
            missile->node->setVisible(false);
            missile->alive = false;
        }

        if (!missile->target->flaresOn) {
            core::vector3df target = (missile->target->node->getPosition() - missile->node->getPosition()).normalize();
            missile->v = target * MISSILE_SPEED * dt;
        }

        core::vector3df missilePos = missile->node->getPosition();
        missilePos += missile->v;
        missile->node->setPosition(missilePos);

        // hit target
        if ((missile->target->node->getPosition() - missilePos).getLength() < 100) {
            explode(missile->target);

            missile->node->setVisible(false);
            missile->alive = false;
            missile->target->node->setVisible(false);
            enemiesAlive--;
        }

        // hit map
        if (missile->collider->collisionOccurred()) {
            missile->node->setVisible(false);
            missile->alive = false;
        }

    }
}

void shootMissile(Missile* missile, Balloon* target, core::vector3df startPos) {
    if (!missile->alive) {
        missile->node->setPosition(startPos);
        missile->target = target;
        missile->alive = true; 
        missile->node->setVisible(true);
    }
}

void systemMissile(Missile* missile, f32 dt) {
    if (missile->alive) {
        core::vector3df target = (missile->target->node->getPosition() - missile->node->getPosition()).normalize();
        
        core::vector3df missilePos = missile->node->getPosition();
        missilePos += target * MISSILE_SPEED * dt;
        missile->node->setPosition(missilePos);

        if (missile->collider->collisionOccurred()) {
            missile->node->setVisible(false);
            missile->alive = false;
            missile->target->node->setVisible(false);
        }
    }
}

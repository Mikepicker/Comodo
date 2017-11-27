bool playerInRange(core::vector3df pos) {
    
    core::vector3df playerPos = player.balloon.node->getPosition();
    core::vector3df hitPoint;
    core::triangle3df triangle;
    core::line3d<f32> ray;
    ray.start = pos;
    ray.end = playerPos;//ray.start + (playerPos - ray.start).normalize() * RAY_LENGTH;
    scene::ISceneNode* selectedSceneNode = collMan->getSceneNodeAndCollisionPointFromRay(ray, hitPoint, triangle, id_player);

    printf("%f\n", (ray.end - ray.start).getLength());
    if (selectedSceneNode && (selectedSceneNode->getID() & id_player) == id_player) {
        printf("SIGHTED!\n");
        return true;

        // set enemyLocked
        /*if (!player.balloon.enemyLocked) {
            player.balloon.enemyLocked = true;
            player.balloon.enemyLockedSince = now;
        } else if (now - player.balloon.enemyLockedSince > LOCK_TIME) {
            
            // Shoot missile
            if (inputReceiver.GetMouseState().LeftButtonDown) {
                Enemy* enemy = findEnemyByName(selectedSceneNode->getName());
                shootMissile(&player.balloon.missile, &(enemy->balloon), player.balloon.node->getPosition());
            }
        }*/
        
    } else {
        player.balloon.enemyLocked = false;
    }

    return false;

}

void systemAI(Enemy* enemy, f32 dt) {

    // wander
    if (enemy->state == "state_idle") {
        enemy->searchPoint = core::vector3df(randInRange(0, 60*MAP_WIDTH), 1000, randInRange(0, sqrt(3)*200*MAP_HEIGHT));
        enemy->state = "state_search";
        return;
    }

    // search and shoot
    if (enemy->state == "state_search") {
        core::vector3df pos = enemy->balloon.node->getPosition();
        s32 dist = (enemy->searchPoint - pos).getLength();

        if (playerInRange(pos)) {
            shootMissile(&enemy->balloon.missile, &(player.balloon), enemy->balloon.node->getPosition());
        }

        if (dist < 10) {
            enemy->state = "state_idle";
            return;
        }

        core::vector3df dir = (enemy->searchPoint - pos).normalize();
        pos += dir * 400 * dt;
        enemy->balloon.node->setPosition(pos);
    }
}

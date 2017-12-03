bool playerInRange(core::vector3df pos) {
   
    core::vector3df playerVel = player.balloon.v;
    core::vector3df playerPos = player.balloon.node->getPosition() - (playerVel.normalize() * 100);
    core::vector3df hitPoint;
    core::triangle3df triangle;
    core::line3d<f32> ray;
    ray.start = pos + (playerPos - pos).normalize()*100;
    ray.end = ray.start + (playerPos - ray.start).normalize() * RAY_LENGTH;
    scene::ISceneNode* selectedSceneNode = collMan->getSceneNodeAndCollisionPointFromRay(ray, hitPoint, triangle);

    if (selectedSceneNode && (selectedSceneNode->getID() & id_player) == id_player) {
//        printf("SIGHTED!\n");
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
 //       printf("NOT SIGHTED!\n");
    }

    return false;

}

void createEnemy() {

    core::stringw balloons[3];
    balloons[0] = "balloon_blue.obj";
    balloons[1] = "balloon_red.obj";
    balloons[2] = "balloon_yellow.obj";

    for (s32 i = 0; i < MAX_ENEMIES; i++) {

        Enemy* enemy = &enemies[i];

        if (!enemy->balloon.alive) {

            s32 rnd = randInRange(0, 2);
            core::stringw mesh = "assets/";
            mesh += balloons[rnd];

            scene::IAnimatedMesh* balloonMesh = smgr->getMesh(mesh);
            core::vector3df pos = core::vector3df(randPosition());

            video::SColor color;
            if (rnd == 0) { color = video::SColor(255, 0, 0, 255); }
            else if (rnd == 1) { color = video::SColor(255, 255, 0, 0); }
            else if (rnd == 2) { color = video::SColor(255, 255, 255, 0); }

            smgr->getMeshManipulator()->setVertexColors(balloonMesh, color);
            
            enemy->balloon.node = smgr->addAnimatedMeshSceneNode(balloonMesh, 0, id_enemy);
            enemy->balloon.node->setPosition(pos);
            scene::ITriangleSelector* selector = smgr->createTriangleSelectorFromBoundingBox(enemy->balloon.node);
            enemy->balloon.node->setTriangleSelector(selector);
            selector->drop();
            
            core::stringw name = "enemy_";
            name += i;

            enemy->balloon.node->setName(name);
            enemy->name = name;
            enemy->state = "state_idle";
            enemy->balloon.jump = false;
            enemy->balloon.enemyLocked = false;
            enemy->balloon.flaresOn = false;
            enemy->balloon.isLocked = false;
            enemy->balloon.flaresStart = FLARES_COOLDOWN;
            enemy->balloon.alive = true;
            enemy->balloon.scale = 1;

            // sensor
            //enemy.sensor = smgr->addSphereSceneNode(200.0f);
            //enemy.balloon.node->addChild(enemy.sensor);
            /*enemy.sensorCollider = smgr->createCollisionResponseAnimator(
                    metaSelector, enemy.balloon.node, core::vector3df(200), core::vector3df(0,0,0));
            enemy.balloon.node->addAnimator(enemy.sensorCollider);*/

            
            attachFlaresPs(&enemy->balloon);
            attachExplosionPs(&enemy->balloon);

            balloonMesh->drop();

            createMissile(&enemy->balloon);

            enemiesAlive++;

            return;

        }
    }

}

void systemAI(f32 dt) {

    const u32 now = device->getTimer()->getTime();

    for (s32 i = 0; i < MAX_ENEMIES; i++) {

        Enemy* enemy = &enemies[i];
        if (enemy->balloon.alive) {

            // evade missile
            if (enemy->balloon.isLocked) {

                
                if (enemy->balloon.flaresOn) {
                    if (now - enemy->balloon.flaresStart > FLARES_TIME) {
                        enemy->balloon.flaresPs->setEmitter(0);
                        enemy->balloon.flaresOn = false;
                    }
                } else {
                    if (now - enemy->balloon.flaresStart >= FLARES_COOLDOWN && now > enemy->balloon.timeToFlares) {
                        enemy->balloon.flaresPs->setEmitter(enemy->balloon.flaresEm);
                        enemy->balloon.flaresOn = true;
                        enemy->balloon.flaresStart = now;
                    }
                }
            }

            // wander
            if (enemy->state == "state_idle") {
                enemy->searchPoint = core::vector3df(randInRange(0, 60*MAP_WIDTH), randInRange(1000, 1500), randInRange(0, sqrt(3)*200*MAP_HEIGHT));
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
                pos += dir * ENEMY_SPEED * dt;
                enemy->balloon.node->setPosition(pos);
            }

            // hide
            if (enemy->state == "state_hide") {
                if (enemy->sensorCollider->collisionOccurred()) {
                    printf("COLLISION!\n");
                }

                core::vector3df pos = enemy->balloon.node->getPosition();
                core::vector3df dir = (player.balloon.node->getPosition() - pos).normalize();
                pos += dir * 20 * dt;
                enemy->balloon.node->setPosition(pos);

            }
        }
    }
}

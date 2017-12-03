void initPlayerData() {
    player.balloon.enemyLocked = false;
    player.balloon.enemyLockedSince = 0;
    player.balloon.flaresStart = FLARES_COOLDOWN;
    player.balloon.alive = true;
    player.balloon.node->setPosition(randPosition());
    player.balloon.node->setVisible(true);
    player.balloon.v = core::vector3df(0);

    scene::ICameraSceneNode* camera = (scene::ICameraSceneNode*)player.balloon.node;
}

void createPlayer() {
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

    player.balloon.node = smgr->addCameraSceneNodeFPS(0, 100, 1, -1, keyMap, 8, true);
    player.balloon.node->setPosition(randPosition());
    ((scene::ICameraSceneNode*)(player.balloon.node))->setFarValue(1000000.0f);
    player.balloon.v = core::vector3df(0, 0, 0);
    player.balloon.node->setName("player");

    // animator (collisions)
    player.balloon.collider = smgr->createCollisionResponseAnimator(
            metaSelector, player.balloon.node, core::vector3df(30, 50, 30),
            core::vector3df(0, 0, 0), core::vector3df(0, 30, 0));

    player.balloon.node->addAnimator(player.balloon.collider);
    metaSelector->drop();

    scene::IAnimatedMeshSceneNode* playerMesh = smgr->addAnimatedMeshSceneNode(balloonMesh, 0, id_player);
    playerMesh->setPosition(core::vector3df(0, 0, -100));
    scene::ITriangleSelector* selector = smgr->createTriangleSelectorFromBoundingBox(playerMesh);
    playerMesh->setTriangleSelector(selector);
    player.balloon.node->addChild(playerMesh);

    balloonMesh->drop();
   
    // init data
    initPlayerData();

    // attach flares
    attachFlaresPs(&player.balloon);
    attachExplosionPs(&player.balloon);
}

void systemPlayer(f32 dt) {

    const u32 now = device->getTimer()->getTime();
    
    scene::ICameraSceneNode* camera = (scene::ICameraSceneNode*)player.balloon.node;

    // player killed
    if (!player.balloon.alive) {
        return;
    }

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
    playerPos.X += v.X;
    playerPos.Y += v.Y;

    // out of bounds
    /*if (playerPos.X < 0) {
        playerPos.X = 1000;
    } else if (playerPos.X > 1000) {
        playerPos.X = 0;
    }

    if (playerPos.Y < -1000) {
        playerPos.Y = 1000;
    } else if (playerPos.Y > 1000) {
        playerPos.Y = 0;
    }

    if (playerPos.Z < 0) {
        playerPos.Z = 1000;
    } else if (playerPos.Z > 1000) {
        playerPos.Z = 0;
    }*/

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

    // collision
    f32 distToDeathSphere = (deathSphere.node->getPosition() - playerPos).getLength();
    if (distToDeathSphere < DEATHSPHERE_RADIUS) {
        player.balloon.alive = false;
        return;
    }

    // get aimed target
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
                enemy->balloon.isLocked = true;
                enemy->balloon.lockedBy = &player.balloon;
                printf("%s\n", enemy->name.c_str());
                shootMissile(&(player.balloon.missile), &(enemy->balloon), player.balloon.node->getPosition());
            }
            
        }
        
    } else {
        player.balloon.enemyLocked = false;
        currentAim = aimWhite;
    }

    // flares
    if (player.balloon.flaresOn) {
        if (now - player.balloon.flaresStart > FLARES_TIME) {
            player.balloon.flaresPs->setEmitter(0);
            player.balloon.flaresOn = false;
        }
    } else {
        if (now - player.balloon.flaresStart > FLARES_COOLDOWN && inputReceiver.GetMouseState().RightButtonDown) {
            player.balloon.flaresPs->setEmitter(player.balloon.flaresEm);
            player.balloon.flaresOn = true;
            player.balloon.flaresStart = now;
        }
    }
}

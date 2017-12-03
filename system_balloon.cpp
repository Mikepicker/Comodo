void systemBalloon(Balloon* balloon, f32 dt) {
    
    const u32 now = device->getTimer()->getTime();

    // explosion
    if (balloon->explosionOn) {
        if (now - balloon->explosionStart > 300) {
            balloon->explosionPs->setEmitter(0);
            balloon->explosionOn = false;
        }
    }
}

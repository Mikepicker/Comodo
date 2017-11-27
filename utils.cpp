s32 randInRange(s32 min, s32 max) {
    return rand() % (max + 1 - min) + min;
}

Enemy* findEnemyByName(std::string name) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].name == name) {
            return &enemies[i];
        }
    }
}

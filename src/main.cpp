#include "Game.h"

int main(int argc, char* argv[]) {
    system("chcp 65001");
    Game* game = Game::getInstance();

    if (game->init()) {
        game->run();
    }

    return 0;
}

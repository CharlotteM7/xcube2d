#include "MyGame.h"
#include "../engine/custom/MyEngineSystem.h"


int main(int argc, char * args[]) {

	try {
		MyEngineSystem engineSystem;
		engineSystem.perlin_init();
		MyGame game;
		game.runMainLoop();
	} catch (EngineException & e) {
		std::cout << e.what() << std::endl;
		getchar();
	}

	return 0;
}

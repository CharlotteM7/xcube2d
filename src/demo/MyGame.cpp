#include "MyGame.h"
#include "../engine/custom/MyEngineSystem.h"
#include <algorithm>


MyGame::MyGame() : AbstractGame(), score(0), lives(3), numKeys(5), gameWon(false), box(5, 5, 30, 30) {
	//Font
	TTF_Font* font = ResourceManager::loadFont("res/fonts/arial.ttf", 72);
	gfx->useFont(font);
	gfx->setVerticalSync(true);
	
	//Background Music 
	Mix_Music* bgMusic = ResourceManager::loadMP3("res/sounds/bgmusic.mp3");
	Mix_PlayMusic(bgMusic, -1);

	// Pass the gfx pointer when creating the MyEngineSystem instance
	mySystem = std::make_shared<MyEngineSystem>(gfx);
	// Generate terrain
	mySystem->generateTerrain();



	for (int i = 0; i < numKeys; i++) {
		std::shared_ptr<GameKey> k = std::make_shared<GameKey>();
		k->isAlive = true;
		k->pos = Point2(getRandom(0, 750), getRandom(0, 550));
		gameKeys.push_back(k);
	}
	
}

MyGame::~MyGame() {}

void MyGame::handleKeyEvents() {

	int speed = 3;
	Dimension2i windowSize = gfx->getCurrentWindowSize();
	int screenWidth = windowSize.w;
	int screenHeight = windowSize.h;

	if (eventSystem->isPressed(Key::W)) {
		if (box.y - speed >= 0) {  // check top boundary
			velocity.y = -speed;
		}
	}

	if (eventSystem->isPressed(Key::S)) {
		if (box.y + speed + box.h <= screenHeight) {  // check bottom boundary
			velocity.y = speed;
		}
	}

	if (eventSystem->isPressed(Key::A)) {
		if (box.x - speed >= 0) {  // check left boundary
			velocity.x = -speed;
		}
	}

	if (eventSystem->isPressed(Key::D)) {
		if (box.x + speed + box.w <= screenWidth) {  // check right boundary
			velocity.x = speed;
		}
	}
}


void MyGame::update() {
	mySystem->playerX += velocity.x;
	mySystem->playerY += velocity.y;

	mySystem->updateVisibleChunks(mySystem->playerX, mySystem->playerY, MyEngineSystem::viewDistance);

	box.x += velocity.x;
	box.y += velocity.y;

	for (int i = 0; i < gameKeys.size(); i++) {
		if (gameKeys[i]->isAlive) {
			int keyX = gameKeys[i]->pos.x - mySystem->playerX ;
			int keyY = gameKeys[i]->pos.y - mySystem->playerY  ;

			Point2 newPos(keyX, keyY);

			if (box.contains(newPos)) {
				score += 200;
				gameKeys[i]->isAlive = false;
				numKeys--;
			}
		}
	}


	velocity.x = 0;
	velocity.y = 0;

	if (numKeys == 0) {
		gameWon = true;
	}
}


void MyGame::render() {
	// Render terrain
	mySystem->renderTerrain();

		//// Render keys
		gfx->setDrawColor(SDL_COLOR_YELLOW);
		for (auto key : gameKeys) {
			if (key->isAlive) {
				int keyX = key->pos.x - mySystem->playerX;
				int keyY = key->pos.y - mySystem->playerY;

				Point2 newPos(keyX, keyY);
				gfx->drawCircle(newPos, 5);
			}
		}

		// Render box
		gfx->setDrawColor(SDL_COLOR_BLACK);
		gfx->fillRect(&box.getSDLRect());
	}



void MyGame::renderUI() {
	gfx->setDrawColor(SDL_COLOR_BLACK);
	std::string scoreStr = std::to_string(score);
	

	gfx->drawText(scoreStr, 780 - scoreStr.length() * 50, 25);
	

	if (gameWon)
		gfx->drawText("YOU WON", 250, 500);
}




#include "MorpheusError.h"
#include "MorpheusWindow.h"
#include "MorpheusCamera.h"
#include "MorpheusGameObject.h"
#include "MorpheusTransform.h"
#include "MorpheusKeys.h"
#include "MorpheusTime.h"
#include "MorpheusShader.h"
#include "MorpheusResourceManager.h"
#include "MorpheusModel.h"
#include "MorpheusNoise.h"
#include <GL/glew.h>
#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <mutex>
#include <thread>
#include <chrono>
#include <math.h>
#include <iostream>
#include <random>
#include <exception>
#include <vld.h>
#define main main

int handleQuit(std::thread* thisThread, double elapsedTime) {
	std::cout << "The program has quit.  " << (elapsedTime / 1000) << " seconds elapsed." << std::endl;
	return 0;
}

int handleKeyChange(std::thread* thisThread, morpheus::KeyHandler keys) {
	char yn = 0;
	while (yn != 'y' && yn != 'n') {
		if (yn != 0){
			std::cout << "Invalid input." << std::endl << std::endl;
		}
		std::cout << "Change key bindngs?  ";
		std::cin >> yn;
	}
	if (yn == 'y'){
		std::cout << "Modify which key? ";
		int k = 0;
		std::cin >> k;

		std::cout << "KEY_BINDINGS MODIFIED >> JK_LOL!" << std::endl;
	}
	return 0;
}

int main(int argc, char** argv) {
	try {
		morpheus::Timer programTime;
		programTime.start();

		/*std::mt19937_64 rng;
		//rng.seed(std::chrono::duration <double, std::milli>(std::chrono::steady_clock::now().time_since_epoch()).count());
		rng.seed(); //not random, whereas above uses time to introduce inital entropy requirement.
		for (int i = 0; i < 10; ++i){ 
			std::cout << rng() << std::endl;
		}*/

		morpheus::Window window;
		morpheus::KeyHandler keys;

		if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		{
			std::cout << "SDL initialization failed. SDL_Error: " << SDL_GetError() << std::endl;
			return 1;
		}

		keys.resetToDefault();

		std::string title = "Morpheus V0.0.0";
		window.create(title, 800, 600);

		morpheus::ResourceManager* resources = new morpheus::ResourceManager("manifest.mf");

		//removeme
		//resources->convertObjModel("monkey_smooth");
		//int d = 0;
		//std::cin >> d; 
		//return 0;
		//endremoveme 

		std::vector<unsigned char> noiseData;
		int seed = 123;
		for (int a = 0; a < 500; a++){
			for (int b = 0; b < 500; b++){
				unsigned char lval = (unsigned char)((morpheus::noise::cubicNoise(a/30.0, b/30.0, 0, seed) + 1) * 127);
				lval /= 100;
				lval %= 2;
				lval *= 255;
				noiseData.push_back(lval);
				noiseData.push_back(lval);
				noiseData.push_back(lval);
				noiseData.push_back(255);
			}
		}

		morpheus::GameObject* go;
		go = new morpheus::GameObject();
		go->addModelByReference(resources->getModel("cube_smooth"));
		go->addUniqueTransform(new morpheus::Transform());
		//go->addTextureByReference(resources->getTexture("sphere_wrapped"));
		go->addUniqueTexture(new morpheus::Texture(&noiseData[0], 500, 500));

		morpheus::Camera camera;
		camera.createPerspectiveProjectionMatrix(800, 600, 0.1f, 1000.0f, glm::radians(90.0f));
		camera.position.z = 100;

		bool paused = false;
		bool rotateMonkey = false;

		window.show();

		bool shouldExit = false;

		SDL_Event event;
		int frameCount = 0;
		int fps = 0;
		morpheus::Timer fpsTime;

		morpheus::Timer frameTime;
		double eventTime = 0;
		double updateTime = 0;
		double renderTime = 0;

		fpsTime.start();
		while (!shouldExit) {
			frameTime.start();
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_QUIT:
					shouldExit = true;
					break;
				case SDL_WINDOWEVENT:
					switch (event.window.event) {
					case SDL_WINDOWEVENT_CLOSE:
						shouldExit = true;
						break;
					}
					break;
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_F2){
						std::thread keyHandler = std::thread(handleKeyChange, &keyHandler, keys);
						keyHandler.join();
					}
					if (event.key.keysym.sym == SDLK_F6){
						static bool isWireFrame = false;
						isWireFrame = !isWireFrame;
						if (isWireFrame){
							glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						}
						else {
							glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
						}
					}
					if (event.key.keysym.sym == SDLK_F8){
						//reset mankey
					}
					if (event.key.keysym.sym == SDLK_F9){
						rotateMonkey = !rotateMonkey;
					}
					if (event.key.keysym.sym == keys.getKey(morpheus::Keys::Up)){
						camera.position.z += 2;
					}
					if (event.key.keysym.sym == keys.getKey(morpheus::Keys::Left)){
						camera.position.x -= 2;
					}
					if (event.key.keysym.sym == keys.getKey(morpheus::Keys::Down)){
						camera.position.z -= 2;
					}
					if (event.key.keysym.sym == keys.getKey(morpheus::Keys::Right)){
						camera.position.x += 2;
					}
				}
			}
			eventTime = frameTime.getMillis();
			if (!shouldExit && !paused){
				if (rotateMonkey){
					go->getTransform()->rotation.y++;
				}
			}
			updateTime = frameTime.getMillis() - eventTime;
			if (!shouldExit){
				window.beginDraw();
				glClearColor(0, 0, 0, 1);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				std::string shaderToUse = "texture_test";
				resources->getShader(shaderToUse)->bind();
				camera.bind(resources->getShader(shaderToUse));
				go->draw(resources->getShader(shaderToUse));
				window.endDraw();
				morpheus::handleError(morpheus::Error::GL_DRAW, true);
			}
			renderTime = frameTime.getMillis() - (updateTime + eventTime);
			frameCount++;
			if (fpsTime.getMillis() >= 1000){
				fps = frameCount;
				frameCount = 0;
				fpsTime.start();
			}
			window.setTitle(title, true, fps, eventTime, updateTime, renderTime, frameTime.getMillis());
		}
		delete go;
		delete resources;
		window.destroy();
		SDL_Quit();
		std::thread quitHandler = std::thread(handleQuit, &quitHandler, programTime.getMillis());
		quitHandler.join();
		//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	catch (std::exception& e){
		morpheus::handleError(morpheus::Error::UNCAUGHT, false);
		std::cout << e.what() << std::endl;
	}
	catch (...) {
		morpheus::handleError(morpheus::Error::UNCAUGHT, false);
	}
	int keytoexit = 0;
	std::cin >> keytoexit;
	return 0;
}
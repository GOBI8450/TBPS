#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <sstream>
#include <iostream>
#include <random>
#include <ctime>
#include "LineLink.h"
#include "ObjectsList.h"
#include "Grid.h" 
#include "Button.h"
#include "Rectangle.h"
#include <boost\asio.hpp>
#include "Options.h"
#include "PhysicsSimulation.h"

//To run SinglePlayer
class PhysicsSimulation_SinglePlayer : public PhysicsSimulationVisual, public PhysicsSimulationActions {
protected:

#pragma region EssantialVariables
	sf::VideoMode desktopSize = sf::VideoMode::getDesktopMode();
	int window_height = desktopSize.height;
	int window_width = desktopSize.width;
	bool fullscreen = false;
	float gravity = 9.8;
	double massLock = 0;


	// Window and view settings
	sf::RenderWindow& window;
	sf::View view;
	sf::ContextSettings settings;
	const float ZOOM_FACTOR = 1.5f;

	// Cursors
	sf::Cursor handCursor;
	sf::Cursor defaultCursor;

	std::string screen = "START";

	bool hovering = false;
	bool connectingMode = false;
	bool createConnectedObjMode = false;
	bool createChain = false;
	bool planetMode = false;
	bool enableCollison = true;
	bool borderless = true;

	// Physics and simulation parameters
	float lineLength = 45;
	ObjectsList objectList;
	float deltaTime = 1.0f / 60.0f;
	float elastic = 0.0;
	int objCount = 0;
	float radius = 50;

	// Mouse and interaction state
	sf::Vector2f* previousMousePos = nullptr;
	sf::Vector2f currentMousePos;
	bool mouseClickFlag = false;
	bool mouseFlagScrollUp = false;
	bool mouseFlagScrollDown = false;
	int mouseScrollPower = 5;
	bool scaleFlag = false;
	bool TouchedOnce = false;
	float moveSpeedScreen = 15.f;
	int oldGravity = gravity;
	bool freeze = false;
	int typeOfLink = 1; //1 -> fixed. 2 -> non fixed. TODO: 3 -> loose/custom by user

	// Object pointers for interaction
	BaseShape* thisBallPointer = nullptr;
	BaseShape* previousBallPointer = nullptr;

	// Visual settings
	sf::Color ball_color = sf::Color(238, 238, 238);
	sf::Color ball_color2 = sf::Color(50, 5, 11);
	sf::Color background_color = sf::Color(30, 30, 30);
	sf::Color buttonColor = sf::Color(55, 58, 64);
	sf::Color bb = sf::Color(44, 55, 100);
	sf::Color explosion = sf::Color(205, 92, 8);
	sf::Color outlineColor = sf::Color(255, 255, 255);
	sf::Color previousColor = sf::Color(0, 0, 0);

	// Gradient settings
	short int gradientStep = 0;
	short int gradientStepMax = 400;
	std::vector<sf::Color> gradient;

	// UI Elements
	sf::Font font;
	sf::Text ballsCountText;
	sf::Text fpsText;
	sf::Text linkingText;

	// Menu elements
	sf::RectangleShape headerText;
	std::vector<std::pair<Button, bool>> settingsButtonVec;

	// Performance tracking
	sf::Clock clock;
	sf::Clock fpsClock;
	int frameCount = 0;
	float currentFPS = 0.0f;

	// Object templates
	Circle* copyObjCir;
	RectangleClass* copyObjRec;

	// Spawn settings
	float posYStartingPoint = 200;
	int posXStartingPoint = radius;
	short int startingPointAdder = 31;
	sf::Vector2f spawnStartingPoint;
	sf::Vector2f initialVel = sf::Vector2f(4, 0);
#pragma endregion

public:
	PhysicsSimulation_SinglePlayer(sf::RenderWindow& win) : PhysicsSimulationVisual(), PhysicsSimulationActions(),
		window(win),
		settings(8),
		objectList(lineLength),
		spawnStartingPoint(posXStartingPoint, posYStartingPoint)
	{
		view = window.getDefaultView();  // Initialize view from window
		initializeCursors();
		loadResources();
		initializeUI();
		setupGradient();
	}

	std::string Run() {
		currentMousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view);
		handleAllEvents();
		renderSimulation();
		return screen;
	}

private:
	void handleEventsFromPollEvent(sf::Event event) override {
		if (event.type == sf::Event::Closed) { window.close(); }
		handleKeyPress(event);
		handleMouseRelase(event);
		handleMouseWheel(event);
	}

	void handleAllEvents() override {
		sf::Event event;
		while (window.pollEvent(event)) {
			handleEventsFromPollEvent(event);
		}
		handleMouseClick();
		handleScaling();
		handleMouseInteraction();
	}

	void handleKeyPress(sf::Event event) override {
		if (event.type == sf::Event::KeyPressed) {
			if (event.key.code == sf::Keyboard::Escape) {
				screen = "MAIN MENU";
				objectList.DeleteAll();
				objCount = 0;
			}
			if (event.key.code == sf::Keyboard::A) {
				for (size_t i = 0; i < 10; i++)
				{
					BaseShape* newObject = objectList.CreateNewCircle(options.gravity, gradient[gradientStep], spawnStartingPoint, initialVel);
					objCount += 1;
					gradientStep += 1;
					spawnStartingPoint.x += startingPointAdder;
					if (gradientStep == gradientStepMax) {
						std::reverse(gradient.begin(), gradient.end());
						gradientStep = 0;
					}
					if (spawnStartingPoint.x >= options.window_width - radius || spawnStartingPoint.x <= radius)
					{
						startingPointAdder *= -1;
					}
					objectList.connectedObjects.AddObject(newObject);
				}
			}
			if (event.key.code == sf::Keyboard::T) {
				for (size_t i = 0; i < 10; i++)
				{
					objectList.CreateNewRectangle(options.gravity, gradient[gradientStep], spawnStartingPoint);
					objCount += 1;
					gradientStep += 1;
					spawnStartingPoint.x += startingPointAdder;
					if (gradientStep == gradientStepMax) {
						std::reverse(gradient.begin(), gradient.end());
						gradientStep = 0;
					}
					if (spawnStartingPoint.x >= options.window_width - radius || spawnStartingPoint.x <= radius)
					{
						startingPointAdder *= -1;
					}
				}
			}
			if (event.key.code == sf::Keyboard::C) {
				if (!connectingMode)
				{
					connectingMode = true;
					previousBallPointer = thisBallPointer;
					linkingText.setString("ACTIVATED");
					linkingText.setFillColor(sf::Color::Magenta);
				}
				else {
					connectingMode = false;
					linkingText.setString("DEACTIVATED");
					linkingText.setFillColor(sf::Color::White);
				}
			}
			if (event.key.code == sf::Keyboard::K) {
				createConnectedObjMode = !createConnectedObjMode;
				previousBallPointer = thisBallPointer;
			}
			if (event.key.code == sf::Keyboard::X) {
				createChain = !createChain;
				previousBallPointer = thisBallPointer;
			}
			if (event.key.code == sf::Keyboard::B && (createConnectedObjMode||createChain) && previousBallPointer!=nullptr) {
				if (createChain)
				{
					previousBallPointer = objectList.createNewLinkedCircle(previousBallPointer, typeOfLink, options.gravity, gradient[gradientStep], currentMousePos, initialVel);
				}
				else
				{
					objectList.createNewLinkedCircle(previousBallPointer, typeOfLink, options.gravity, gradient[gradientStep], currentMousePos, initialVel);
				}
				objCount += 1;
			}
			if (event.key.code == sf::Keyboard::BackSpace && mouseClickFlag) {
				objectList.DeleteThisObj(thisBallPointer);
			}
			if (event.key.code == sf::Keyboard::H) {
				for (size_t i = 0; i < 1; i++)
				{
					BaseShape* newObject = objectList.CreateNewCircle(options.gravity, gradient[gradientStep], currentMousePos, initialVel);
					objCount += 1;
					objectList.connectedObjects.AddObject(newObject);
					objectList.connectedObjects.ConnectRandom(10, typeOfLink);
				}
			}
			if (event.key.code == sf::Keyboard::F11)
			{
				if (!options.fullscreen)
				{
					window.create(options.desktopSize, "Fullscreen Mode", sf::Style::Fullscreen);
					options.fullscreen = true;
				}
				else {
					window.create(sf::VideoMode(1920, 980), "Tomy Mode", sf::Style::Default);
					options.fullscreen = false;
				}
			}
			if (event.key.code == sf::Keyboard::L) {
				createPlanet();
			}
			if (event.key.code == sf::Keyboard::F) {
				objectList.CreateNewCircle(options.gravity, explosion, sf::Vector2f(currentMousePos.x + 3, currentMousePos.y + 3), initialVel);
				for (size_t i = 0; i < 50; i++)
				{
					objectList.CreateNewCircle(options.gravity, explosion, currentMousePos, initialVel);
					objCount += 1;
				}
			}
			if (event.key.code == sf::Keyboard::J) {
				sf::Vector2i mousePos = sf::Mouse::getPosition(window);
				objectList.CreateNewRectangle(options.gravity, explosion, sf::Vector2f(currentMousePos.x + 3, currentMousePos.y + 3));
				for (size_t i = 0; i < 50; i++)
				{
					objectList.CreateNewRectangle(options.gravity, explosion, currentMousePos);
					objCount += 1;
				}
			}
			if (event.key.code == sf::Keyboard::R) {
				objectList.DeleteAll();
				objCount = 0;
			}
			if (event.key.code == sf::Keyboard::S && mouseClickFlag)
			{
				scaleFlag = true;
			}
			if (event.key.code == sf::Keyboard::Space)
			{
				if (!freeze)
				{
					options.gravity = 0;
					freeze = true;
				}
				else
				{
					freeze = false;
					options.gravity = oldGravity;
				}
			}
			if (event.key.code == sf::Keyboard::Num1)
			{
				typeOfLink = 1;
			}
			if (event.key.code == sf::Keyboard::Num2)
			{
				typeOfLink = 2;
			}
			if (event.key.code == sf::Keyboard::Left)
				view.move(-moveSpeedScreen, 0.f);
			if (event.key.code == sf::Keyboard::Right)
				view.move(moveSpeedScreen, 0.f);
			if (event.key.code == sf::Keyboard::Up)
				view.move(0.f, -moveSpeedScreen);
			if (event.key.code == sf::Keyboard::Down)
				view.move(0.f, moveSpeedScreen);
		}
	}

	void handleMouseClick() override {
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && mouseClickFlag == false) {
			thisBallPointer = objectList.IsInRadius(currentMousePos); // Check if a circle is within radius
			if (previousBallPointer == nullptr)
			{
				previousBallPointer = thisBallPointer;
			}
			if (thisBallPointer != nullptr) { // Check if a circle was found
				mouseClickFlag = true; // Set flag if circle found
				window.setMouseCursor(handCursor);
			}
		}
	}

	void handleMouseRelase(sf::Event event) override {
		if (event.type == sf::Event::MouseButtonReleased) {
			mouseClickFlag = false;
			scaleFlag = false;
			if (thisBallPointer != nullptr)
			{
				window.setMouseCursor(defaultCursor);
				thisBallPointer->setColor(previousColor);
				thisBallPointer->SetOutline(outlineColor, 0);
			}
			TouchedOnce = false;
		}
	}

	void handleMouseInteraction() override {
		if (mouseClickFlag) { // Check if a circle is found
			thisBallPointer->SetPosition(currentMousePos); // Set position of the found ball
			if (!TouchedOnce)
			{
				thisBallPointer->SetOutline(outlineColor, 5);
				// Get the current color
				sf::Color currentColor = thisBallPointer->GetColor();
				previousColor = currentColor;
				// Darken the color by reducing the RGB values (without going below 0)
				currentColor.r = std::max(0, currentColor.r - 15);
				currentColor.g = std::max(0, currentColor.g - 15);
				currentColor.b = std::max(0, currentColor.b - 15);

				// Apply the new color to the rectangle
				thisBallPointer->setColor(currentColor);
				TouchedOnce = true;
			}
			else if (connectingMode && thisBallPointer != previousBallPointer)
			{
				objectList.connectedObjects.MakeNewLink(previousBallPointer, thisBallPointer, typeOfLink);
			}
			handleScaling();
		}
	}

	void handleMouseWheel(sf::Event event) override {
		if (event.type == sf::Event::MouseWheelScrolled) {
			if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
				//sf::Vector2f beforeZoom = window.mapPixelToCoords(sf::Vector2i(event.mouseWheelScroll.x, event.mouseWheelScroll.y), view);
				if (event.mouseWheelScroll.delta > 0) {

					if (!scaleFlag) {
						view.zoom(1.f / ZOOM_FACTOR);
					}
					mouseFlagScrollUp = true;
				}

				else if (event.mouseWheelScroll.delta < 0) {
					if (!scaleFlag) {
						view.zoom(ZOOM_FACTOR);
					}
					mouseFlagScrollDown = true;
				}

				//sf::Vector2f afterZoom = window.mapPixelToCoords(sf::Vector2i(event.mouseWheelScroll.x, event.mouseWheelScroll.y), view);
				//sf::Vector2f offset = beforeZoom - afterZoom;
				//view.move(offset);
			}
		}
	}

	void toggleConnectingMode() override {
		connectingMode = !connectingMode;
		linkingText.setString(connectingMode ? "ACTIVATED" : "DEACTIVATED");
		linkingText.setFillColor(connectingMode ? sf::Color::Magenta : sf::Color::White);
	}

	void createConnectedObjects() override {
		for (size_t i = 0; i < 1; i++) {
			BaseShape* newObject = objectList.CreateNewCircle(options.gravity, gradient[gradientStep], currentMousePos, initialVel);
			objCount++;
			objectList.connectedObjects.AddObject(newObject);
			objectList.connectedObjects.ConnectRandom(10, typeOfLink);
		}
	}

	void createPlanet() override {
		planetMode = true;
		objectList.CreateNewPlanet(7000, ball_color, currentMousePos, 20, 5.9722 * pow(10, 15));
		objCount++;
	}

	void handleScaling() override {
		if (scaleFlag && (mouseFlagScrollUp || mouseFlagScrollDown)) {
			if (Circle* circle = dynamic_cast<Circle*>(thisBallPointer)) {
				scaleCircle(circle);
			}
			else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(thisBallPointer))
			{
				scaleRectangle(rectangle);
			}
		}
	}

	void scaleCircle(Circle* circle) override {
		if (mouseFlagScrollDown && circle->getRadius() > 0.0001) {
			circle->SetRadiusAndCenter(circle->getRadius() - mouseScrollPower);
			circle->SetMass(circle->GetMass() - mouseScrollPower * 10);
			std::cout << circle->getRadius();
			mouseFlagScrollDown = false;
		}
		else {
			circle->SetRadiusAndCenter(circle->getRadius() + mouseScrollPower);
			circle->SetMass(circle->GetMass() + mouseScrollPower * 10);
			mouseFlagScrollUp = false;
		}
	}

	void scaleRectangle(RectangleClass* rectangle) override {
		if (mouseFlagScrollDown && rectangle->GetHeight() > 0.0001 && rectangle->GetWidth() > 0.0001) {
			rectangle->SetSizeAndOrigin(rectangle->GetWidth() - mouseScrollPower, rectangle->GetHeight() - mouseScrollPower);
			rectangle->SetMass(rectangle->GetMass() - mouseScrollPower * 10);
			mouseFlagScrollDown = false;
		}
		else {
			rectangle->SetSizeAndOrigin(rectangle->GetWidth() + mouseScrollPower, rectangle->GetHeight() + mouseScrollPower);
			rectangle->SetMass(rectangle->GetMass() + mouseScrollPower * 10);
			mouseFlagScrollUp = false;
		}
	}

	void handleConnecting() override {
		if (connectingMode && thisBallPointer != previousBallPointer) {
			objectList.connectedObjects.MakeNewLink(previousBallPointer, thisBallPointer, typeOfLink);
		}
	}

	void createExplosionCircles() override {
		objectList.CreateNewCircle(options.gravity, explosion, sf::Vector2f(currentMousePos.x + 3, currentMousePos.y + 3), initialVel);
		for (size_t i = 0; i < 50; i++) {
			objectList.CreateNewCircle(options.gravity, explosion, currentMousePos, initialVel);
			objCount++;
		}
	}

	void createExplosionRectangles() override {
		objectList.CreateNewRectangle(options.gravity, explosion, sf::Vector2f(currentMousePos.x + 3, currentMousePos.y + 3));
		for (size_t i = 0; i < 50; i++) {
			objectList.CreateNewRectangle(options.gravity, explosion, currentMousePos);
			objCount++;
		}
	}

	//visuals
	void renderSimulation() override {
		updateFPS();
		window.clear(background_color);
		window.setView(view);

		MoveAndDrawObjects();

		window.setView(window.getDefaultView());
		renderTexts();

		window.display();

		limitFrameRate();
	}

	void MoveAndDrawObjects() override {
		if (!freeze)
		{
			objectList.MoveObjects(window_width, window_height, currentFPS, elastic, planetMode, enableCollison, borderless);

		}
		else
		{
			objectList.MoveWhenFreeze(window_width, window_height, currentFPS, borderless);
		}

		objectList.DrawObjects(window, currentFPS, planetMode);
	}

	void initializeUI() override {
		setupText();
		setupHeaders();
	}

	void initializeCursors() override {
		if (!defaultCursor.loadFromSystem(sf::Cursor::Arrow) ||
			!handCursor.loadFromSystem(sf::Cursor::Hand)) {
			throw std::runtime_error("Failed to load cursors");
		}
		window.setMouseCursor(defaultCursor);
	}

	void loadResources() override {
		if (!font.loadFromFile("visuals/font.ttf")) {
			throw std::runtime_error("Failed to load font");
		}
		loadTextures();
	}

	void setupGradient() override {
		sf::Color startColor(128, 0, 128);  // purple
		sf::Color endColor(0, 0, 255);      // blue
		gradient = GenerateGradient(startColor, endColor, gradientStepMax);
	}

	std::vector<sf::Color> GenerateGradient(sf::Color startColor, sf::Color endColor, int steps) override {
		std::vector<sf::Color> gradient;
		float stepR = (endColor.r - startColor.r) / static_cast<float>(steps - 1);
		float stepG = (endColor.g - startColor.g) / static_cast<float>(steps - 1);
		float stepB = (endColor.b - startColor.b) / static_cast<float>(steps - 1);

		for (int i = 0; i < steps; ++i) {
			gradient.push_back(sf::Color(
				startColor.r + stepR * i,
				startColor.g + stepG * i,
				startColor.b + stepB * i
			));
		}
		return gradient;
	}

	void loadTextures() override {
		// Since no textures are used in the original code, this method is kept minimal
		// but provides a hook for future texture loading if needed
		try {
			// Currently no textures are loaded as the simulation uses basic shapes
			// If textures are needed in the future, they can be loaded here:
			/*
			sf::Texture texture;
			if (!texture.loadFromFile("path/to/texture.png")) {
				throw std::runtime_error("Failed to load texture");
			}
			*/
		}
		catch (const std::exception& error) {
			std::cerr << "Texture loading error: " << error.what() << std::endl;
		}
	}

	void updateFPS() override {
		frameCount++;
		float timeElapsed = fpsClock.getElapsedTime().asSeconds();

		if (timeElapsed >= 1.0f) {
			currentFPS = frameCount / timeElapsed;
			frameCount = 0;
			fpsClock.restart();
		}
	}

	void setupText() override {
		// FPS Text
		fpsText.setFont(font);
		fpsText.setCharacterSize(20);
		fpsText.setFillColor(sf::Color::White);
		fpsText.setPosition(10, 10);

		// Balls Count Text
		ballsCountText.setFont(font);
		ballsCountText.setCharacterSize(20);
		ballsCountText.setFillColor(sf::Color::White);
		ballsCountText.setPosition(10, 40);

		// Linking Text
		linkingText.setFont(font);
		linkingText.setString("DEACTIVATED");
		linkingText.setCharacterSize(20);
		linkingText.setFillColor(sf::Color::White);
		linkingText.setPosition(10, 70);
	}

	void setupHeaders() override {
		headerText.setSize(sf::Vector2f(400.f, 100.f));
		headerText.setPosition(
			options.window_width / 2.f - headerText.getSize().x / 2.f,
			100.f
		);
		headerText.setFillColor(buttonColor);
	}

	void renderTexts() override {
		std::ostringstream fpsStream;
		std::ostringstream ballCountStream;

		fpsStream << "FPS: " << static_cast<int>(currentFPS);
		ballCountStream << "Balls Count: " << static_cast<int>(objCount);

		fpsText.setString(fpsStream.str());
		ballsCountText.setString(ballCountStream.str());

		window.draw(fpsText);
		window.draw(linkingText);
		window.draw(ballsCountText);
	}

	void limitFrameRate() override {
		sf::Time elapsed = clock.restart();
		if (elapsed.asSeconds() < deltaTime) {
			sf::sleep(sf::seconds(deltaTime - elapsed.asSeconds()));
		}
	}

	std::vector<BaseShape> ConvertForSending() override {
		return std::vector<BaseShape>();
	}
};

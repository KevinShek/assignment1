#include "Game.h"

#include <iostream>
#include <fstream>
#include <algorithm>

Game::Game(const std::string &config)
{
	init(config);
}

void Game::init(const std::string &path)
{
	// variables for reading
	std::string rowType;
	int winWidth = 1280, winHeight = 720, winFramerate = 60, winFullscreen = 0;
	std::string fontPath;
	int fontSize, fontR, fontG, fontB;

	std::ifstream fin(path);
	if (!fin.is_open())
	{
		std::cerr << "Config file not found!";
		exit(-1);
	}
	while (fin >> rowType)
	{
		if (rowType == "Window") {
			fin >> winWidth >> winHeight >> winFramerate >> winFullscreen;
		}
		else if (rowType == "Font") {
			fin >> fontPath >> fontSize >> fontR >> fontG >> fontB;
			if (!m_font.loadFromFile("../"+fontPath))
			{
				// if we can't load the font, print an error to the error console and exit
				std::cerr << "Could not load font!\n";
				exit(-1);
			}
			m_text.setFont(m_font);
			m_text.setFillColor(sf::Color(sf::Uint8(fontR), sf::Uint8(fontG), sf::Uint8(fontB)));
			m_text.setCharacterSize(fontSize);
		}
		// use the premade PlayerConfig, EnemyConfig, BulletConfig variables in Game.h
		else if (rowType == "Player") {
			fin >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB 
				>> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
		}
		else if (rowType == "Enemy") {
			fin >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG 
				>> m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SP;
		}
		else if (rowType == "Bullet") {
			fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB 
				>> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
		}
	}
	fin.close();

	// set up default window parameters
	m_window.create(sf::VideoMode(winWidth, winHeight), "Assignment 2");
	if (winFullscreen == 1) {
		m_window.create(sf::VideoMode::getFullscreenModes()[0], "Assignment 2", sf::Style::Fullscreen);
	}
	m_window.setFramerateLimit(winFramerate);
	ImGui::SFML::Init(m_window);

	// scale the imgui ui and text size by 2
	ImGui::GetStyle().ScaleAllSizes(2.0f);
	ImGui::GetIO().FontGlobalScale = 2.0f;

	spawnPlayer();
}

std::shared_ptr<Entity> Game::player()
{
	auto &&players = m_entities.getEntities("player");

	return players.front();
}

void Game::run()
{
	// some systems should function while paused (rendering)
	// some systems shouldn't (movement / input)
	while (m_running)
	{
		// update the entity manager
		m_entities.update();

		// required update call to imgui
		ImGui::SFML::Update(m_window, m_deltaClock.restart());

		if (m_paused == false) {
			sEnemySpawner();
			sMovement();
			sCollision();
			sLifespan();
		}
		sUserInput();
		sGUI();
		sRender();

		// incurment the current frame
		// may need to be moved when paused implemented
		m_currentFrame++;
	}
}

void Game::setPaused(bool paused)
{
	m_paused = paused;
}

// respawn the player in the middle of the screen
void Game::spawnPlayer()
{
	// We create every entity by calling EntityManager.addEntity(tag)
	// This returns a stf::shared_ptr<Entity> so we use "auto" to saw typing
	auto entity = m_entities.addEntity("player");

	// Give this entity a Tranform so it spawns at (200,200) with velocity (1,1) and angle (0)
	entity->add<CTransform>(Vec2f(m_window.getSize().x/2, m_window.getSize().y/2), Vec2f(m_playerConfig.S, m_playerConfig.S), 0.0f);

	// The entity's shape will have radius 32, 8 sides, dark grey fill and red outline of thickness of 4
	entity->add<CShape>(m_playerConfig.SR, m_playerConfig.V, sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);
	entity->add<CCollision>(m_playerConfig.CR);
	entity->add<CLifespan>(0);
	// Add an input component to the player so that we can use inputs
	entity->add<CInput>();
}

// spawn an enemy at a random position
void Game::spawnEnemy()
{
	// the enemy must be spawned completely within the bounds of the window
	int randomXOfBound = m_enemyConfig.CR + (rand() % (1 + (m_window.getSize().x - m_enemyConfig.CR) - m_enemyConfig.CR));
	int randomYOfBound = m_enemyConfig.CR + (rand() % (1 + (m_window.getSize().y - m_enemyConfig.CR) - m_enemyConfig.CR));
	//float randomSpeedAdjacentOfBound = -m_enemyConfig.SMIN + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (m_enemyConfig.SMAX - -m_enemyConfig.SMIN)));
	//float randomSpeedOppositeOfBound = -m_enemyConfig.SMIN + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (m_enemyConfig.SMAX - -m_enemyConfig.SMIN)));
	float randomSpeedOfBound = m_enemyConfig.SMIN + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (m_enemyConfig.SMAX - m_enemyConfig.SMIN)));
	float randomAngleOfBound = (0 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (360 - 0)))) * (3.14159265359/180);
	//float randomSpeedOppositeOfBound = randomSpeedAdjacentOfBound * std::tan(randomAngleOfBound);
	float SpeedAdjacent = randomSpeedOfBound * cos(randomAngleOfBound);
	float SpeedOpposite = randomSpeedOfBound * sin(randomAngleOfBound);
	int randomVecticesOfBound = m_enemyConfig.VMIN + (rand() % (1 + (m_enemyConfig.VMAX) - m_enemyConfig.VMIN));
	int randomColorROfBound = 0 + (rand() % (1 + (255) - 0));
	int randomColorGOfBound = 0 + (rand() % (1 + (255) - 0));
	int randomColorBOfBound = 0 + (rand() % (1 + (255) - 0));

	//std::cout << randomXOfBound << " " << randomYOfBound << " " << randomSpeedOfBound << " " << randomVecticesOfBound << std::endl;
	//std::cout << randomXOfBound << " " << randomYOfBound << " " << SpeedAdjacent << " " << SpeedOpposite << std::endl;

	auto entity = m_entities.addEntity("enemy");
	// spawn location and speed
	entity->add<CTransform>(Vec2f(randomXOfBound, randomYOfBound), Vec2f(SpeedAdjacent, SpeedOpposite), randomAngleOfBound);
	// enemy properities
	entity->add<CShape>(m_enemyConfig.SR, randomVecticesOfBound, sf::Color(randomColorROfBound, randomColorGOfBound, randomColorBOfBound), sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), m_enemyConfig.OT);
	entity->add<CCollision>(m_enemyConfig.CR);
	entity->add<CLifespan>(0);
	// record when the most recent enemy was spawned
	m_lastEnemySpawnTime = m_currentFrame;
}

// spawns the small enemies when a bug one (input entity e) explodes
void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
	auto& bigEnemyTrans = e->get<CTransform>();
	auto& bigEnemyShape = e->get<CShape>();
	// TODO: spawn small enemies at the location of the input enemy e
	auto smallEnemy = m_entities.addEntity("senemy");
	smallEnemy->add<CTransform>(Vec2f(bigEnemyTrans.pos.x, bigEnemyTrans.pos.y), Vec2f(bigEnemyTrans.velocity.x, bigEnemyTrans.velocity.y), 0.f);
	smallEnemy->add<CShape>(m_enemyConfig.SR/2, bigEnemyShape.circle.getPointCount(), bigEnemyShape.circle.getFillColor(), bigEnemyShape.circle.getOutlineColor(), bigEnemyShape.circle.getOutlineThickness());
	smallEnemy->add<CLifespan>(m_enemyConfig.L);
	// when we create the smaller enemy, we have to read the values of the original enemy
	// - spawn a number of small enemies equal to the vertices of the original enemy
	// - set each small enemy to the same color as the original, half the size
	// - small enemies are worth double points of the original enemy
}

// spawns a bullet from a given entity to a target location
void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2f &target)
{
	auto bullet = m_entities.addEntity("bullet");
	// do not use std::atan as it only give you half the possible directions 
	float angleLocation = std::atan2((target.y - entity->get<CTransform>().pos.y),(target.x - entity->get<CTransform>().pos.x));
	float SpeedAdjacent = m_bulletConfig.S * cos(angleLocation);
	float SpeedOpposite = m_bulletConfig.S * sin(angleLocation);
	bullet->add<CTransform>(Vec2f(entity->get<CTransform>().pos.x, entity->get<CTransform>().pos.y), Vec2f(SpeedAdjacent, SpeedOpposite), angleLocation);
	bullet->add<CShape>(m_bulletConfig.SR, m_bulletConfig.V, sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB), sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), m_bulletConfig.OT);
	bullet->add<CCollision>(m_bulletConfig.CR);
	bullet->add<CLifespan>(m_bulletConfig.L);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
	// TODO: implement your own special weapon
}

void Game::sMovement()
{
	// TODO: implement all entity movement in this function
	// you should read the m_player->cInput component to determine if the player is moving
	auto &transform = player()->get<CTransform>();
	// the 2.f comes from the fact you are moving 1 unit towards x axis and y axis meaning when you put that into pythagorean theorrem gives you sqrt(2.f) and you divide by it to reach the disired speed of 5 units per frame even diagonally.
	//float length = sqrt((transform.velocity.x * transform.velocity.x) + (transform.velocity.y * transform.velocity.y))/ sqrt(2.f);
	//if (player()->get<CInput>().up == true && player()->get<CInput>().left == true) {
	//	transform.pos.x -= length;
	//	transform.pos.y -= length;
	//}
	//else if (player()->get<CInput>().up == true && player()->get<CInput>().right == true) {
	//	transform.pos.x += length;
	//	transform.pos.y -= length;
	//}
	//else if (player()->get<CInput>().down == true && player()->get<CInput>().right == true) {
	//	transform.pos.x += length;
	//	transform.pos.y += length;
	//}
	//else if (player()->get<CInput>().down == true && player()->get<CInput>().left == true) {
	//	transform.pos.x -= length;
	//	transform.pos.y += length;
	//}
	//else if (player()->get<CInput>().up == true) {transform.pos.y -= transform.velocity.y;}
	//else if (player()->get<CInput>().left == true) {transform.pos.x -= transform.velocity.x;}
	//else if (player()->get<CInput>().down == true) {transform.pos.y += transform.velocity.y;}
	//else if (player()->get<CInput>().right == true) {transform.pos.x += transform.velocity.x;}

	// this was created as a local because this will reset the movement when nothing has been pressed
	Vec2f playerDirection(0.0, 0.0);

	// treating as moving 1 unit of direction
	if (player()->get<CInput>().up) { playerDirection.y -= 1.0; }
	if (player()->get<CInput>().left) { playerDirection.x -= 1.0; }
	if (player()->get<CInput>().down) { playerDirection.y += 1.0; }
	if (player()->get<CInput>().right) { playerDirection.x += 1.0; }

	//std::cout << player()->get<CInput>().up << " " << player()->get<CInput>().left << std::endl;

	if (playerDirection.x != 0.f || playerDirection.y != 0.f) {
		//grab the actual direction of travel
		float length = std::sqrt(playerDirection.x * playerDirection.x + playerDirection.y * playerDirection.y);
		// divide by the actual direction to normalise it
		playerDirection /= length;
		// multiple by the speed to get the speed to move 1 unit of space in the game 
		playerDirection *= m_playerConfig.S;
		// add to the player's position to represent that movement
		transform.pos += playerDirection;
	}

	// prevent the player from going out of bounds
	transform.pos.x = std::max(static_cast<float>(m_playerConfig.SR), std::min(transform.pos.x, static_cast<float>(m_window.getSize().x - m_playerConfig.SR)));
	transform.pos.y = std::max(static_cast<float>(m_playerConfig.SR), std::min(transform.pos.y, static_cast<float>(m_window.getSize().y - m_playerConfig.SR)));

	// enemy movement and wall collision
	for (auto& enemy : m_entities.getEntities("enemy")) {
		auto &enemyMovement = enemy->get<CTransform>();

		enemyMovement.pos += enemyMovement.velocity;

		// Bounce off right and left window edges
		if (m_enemyConfig.SR + enemyMovement.pos.x > m_window.getSize().x || enemyMovement.pos.x - m_enemyConfig.SR  < 0) {
			enemyMovement.velocity.x = -enemyMovement.velocity.x;
		}
		// Bounce off bottom and top window edges
		if (m_enemyConfig.SR + enemyMovement.pos.y > m_window.getSize().y || enemyMovement.pos.y - m_enemyConfig.SR < 0) {
			enemyMovement.velocity.y = -enemyMovement.velocity.y;
		}
	}

	// bullet movement
	for (auto& bullet : m_entities.getEntities("bullet")) {
		auto& bulletMovement = bullet->get<CTransform>();
		bulletMovement.pos += bulletMovement.velocity;
		//std::cout << bulletMovement.pos.x << std::endl;
	}
}

void Game::sLifespan()
{
	// TODO: implement all lifespan functionality
	for (auto& entity : m_entities.getEntities()) {
		auto& entityShape = entity->get<CShape>();
		auto& entityLifeSpan = entity->get<CLifespan>();
		if (entityLifeSpan.remaining > 0) {
			if (entityLifeSpan.remaining != 0) {
				entityLifeSpan.remaining --;
				float ratio = static_cast<float>(entityLifeSpan.remaining) / static_cast<float>(entityLifeSpan.lifespan);
				float transparentValue = ratio * 255.f;  // fades out
				entity->get<CShape>().circle.setFillColor(sf::Color(entityShape.circle.getFillColor().r, entityShape.circle.getFillColor().g, entityShape.circle.getFillColor().b, transparentValue));
				entity->get<CShape>().circle.setOutlineColor(sf::Color(entityShape.circle.getOutlineColor().r, entityShape.circle.getOutlineColor().g, entityShape.circle.getOutlineColor().b, transparentValue));
			}
			else {
				entity->destroy();
			}
		}
	}
	// 
	// for all entities
	//		if entity has no lifespan component, skip it
	//		if entity has > 0 remaining lifespan, subtract 1
	//		if it has lifespan and is alive
	//			scale its alpha channel properly
	//		if it has lifespan and its time is up
	//			destroy the entity
}

void Game::sCollision()
{
	// TODO: implement all proper collisions between entities
	// be sure to use the collision radius, NOT the shape radius
	for (auto& bullet : m_entities.getEntities("bullet")) {
		auto& bulletProp = bullet->get<CTransform>();
		//bulletProp.pos
		for (auto& enemy : m_entities.getEntities("enemy")) {
			auto& enemyProp = enemy->get<CTransform>();
			Vec2f vectorOfEntities = { (enemyProp.pos.x - bulletProp.pos.x), (enemyProp.pos.y - bulletProp.pos.y) };
			float distanceOfEntities = std::sqrt(vectorOfEntities.x * vectorOfEntities.x + vectorOfEntities.y * vectorOfEntities.y);
			if (bullet->get<CCollision>().radius + enemy->get<CCollision>().radius > distanceOfEntities) {
				enemy->destroy();
				bullet->destroy();
			}
		}
	}
	for (auto& player : m_entities.getEntities("player")) {
		for (auto& enemy : m_entities.getEntities("enemy")) {
			auto& enemyProp = enemy->get<CTransform>();
			Vec2f vectorOfEnemyandPlayer = { (enemyProp.pos.x - player->get<CTransform>().pos.x), (enemyProp.pos.y - player->get<CTransform>().pos.y) };
			float distanceOfEnemyandPlayer = std::sqrt(vectorOfEnemyandPlayer.x * vectorOfEnemyandPlayer.x + vectorOfEnemyandPlayer.y * vectorOfEnemyandPlayer.y);
			if (player->get<CCollision>().radius + enemy->get<CCollision>().radius > distanceOfEnemyandPlayer) {
				enemy->destroy();
				player->destroy();
			}
		}
	}
	if (player().get()->isActive() == false) {
		spawnPlayer();
	}
}

void Game::sEnemySpawner()
{
	// TODO: code which implements enemy spawning should go here
	if (m_currentFrame % m_enemyConfig.SP == 0)
	{
		spawnEnemy();
	}
}

void Game::sGUI()
{
	ImGui::Begin("Geometry Wars");

	ImGui::Text("Stuff Goes Here");

	ImGui::End();
}

void Game::sRender()
{
	// Draw ALL of the entities
	m_window.clear();

	for (auto& entity : m_entities.getEntities()) {
		entity->get<CShape>().circle.setPosition(entity->get<CTransform>().pos);
		entity->get<CTransform>().angle += 1.0f;
		entity->get<CShape>().circle.setRotation(entity->get<CTransform>().angle);
		m_window.draw(entity->get<CShape>().circle);
	}
	// draw the ui last
	ImGui::SFML::Render(m_window);

	m_window.display();
}

void Game::sUserInput()
{
	// TODO: handle user input here
	// note that you should only be setting the player's input component variables here
	// you should not implement the player's movement logic here
	// the movement system will read the variables you set in this function

	sf::Event event;
	while (m_window.pollEvent(event))
	{
		// pass the event to imgui to be parsed
		ImGui::SFML::ProcessEvent(m_window, event);

		// this event triggers when the window is closed
		if (event.type == sf::Event::Closed)
		{
			m_running = false;
		}

		// this checks if the key is still pressed
		auto& input = player()->add<CInput>();
		input.up = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
		input.left = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
		input.down = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
		input.right = sf::Keyboard::isKeyPressed(sf::Keyboard::D);

		//this event is triggered when a key is pressed
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P)
		{
			setPaused(!m_paused);
		}

		if (event.type == sf::Event::MouseButtonPressed)
		{
			// this line ignores mouse events if ImGui is the thing being clicked
			if (ImGui::GetIO().WantCaptureMouse)
			{
				continue;
			}

			if (event.mouseButton.button == sf::Mouse::Left)
			{
				std::cout << "Left Mouse Button Clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")\n";
				input.shoot = true;
				spawnBullet(player(), Vec2f(event.mouseButton.x, event.mouseButton.y));
			}

			if (event.mouseButton.button == sf::Mouse::Right)
			{
				std::cout << "Right Mouse Button Clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")\n";
				// call spawnSpecialWeapon here
			}
		}
	}
}
#include "Game.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

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
		// ensure the player if destroyed will respawn
		if (player().get()->isActive() == false) {
			spawnPlayer();
		}

		// update the entity manager
		m_entities.update();

		// required update call to imgui
		ImGui::SFML::Update(m_window, m_deltaClock.restart());

		if (m_paused == false) {
			if (m_guiConfig.activeSpawning) {
				sEnemySpawner();
			}
			if (m_guiConfig.activeMovement) {
				sMovement();
			}
			if (m_guiConfig.activeCollision) {
				sCollision();
			}
			if (m_guiConfig.activeLifespan) {
				sLifespan();
			}
		}
		sUserInput();
		if (m_guiConfig.activeGUI) {
			sGUI();
		}
		if (m_guiConfig.activeRendering) {
			sRender();
		}
		else {
			ImGui::SFML::Render(m_window);
			m_window.display();
		}

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
	entity->add<CTransform>(Vec2f(m_window.getSize().x/2, m_window.getSize().y/2), Vec2f(0.0f, 0.0f), 0.0f);

	// The entity's shape will have radius 32, 8 sides, dark grey fill and red outline of thickness of 4
	entity->add<CShape>(m_playerConfig.SR, m_playerConfig.V, sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);
	entity->add<CCollision>(m_playerConfig.CR);
	//entity->add<CLifespan>(0);
	// Add an input component to the player so that we can use inputs
	entity->add<CInput>();
}

// spawn an enemy at a random position
void Game::spawnEnemy()
{
	
	// the enemy must be spawned completely within the bounds of the window and not on top of player position
	float minDistance = 10.f;
	Vec2f distToPlayer;
	int randomXOfBound = 0, randomYOfBound = 0;

	do {
		randomXOfBound = m_enemyConfig.CR + (rand() % (1 + (m_window.getSize().x - m_enemyConfig.CR) - m_enemyConfig.CR));
		randomYOfBound = m_enemyConfig.CR + (rand() % (1 + (m_window.getSize().y - m_enemyConfig.CR) - m_enemyConfig.CR));
		distToPlayer.x = randomXOfBound - player()->get<CTransform>().pos.x;
		distToPlayer.y = randomYOfBound - player()->get<CTransform>().pos.y;
	} while ((std::sqrt((distToPlayer.x * distToPlayer.x) + (distToPlayer.y * distToPlayer.y))) < minDistance);
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
	//entity->add<CLifespan>(0);
	entity->add<CScore>(randomVecticesOfBound*100);
	// record when the most recent enemy was spawned
	m_lastEnemySpawnTime = m_currentFrame;
}

// spawns the small enemies when a bug one (input entity e) explodes
void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
	auto& bigEnemyTrans = e->get<CTransform>();
	auto& bigEnemyShape = e->get<CShape>();
	//  spawn small enemies at the location of the input enemy e
	int numberOfEdges = bigEnemyShape.circle.getPointCount();

	for (int numberOfEdge = 0; numberOfEdge < numberOfEdges; numberOfEdge++) {
		auto smallEnemy = m_entities.addEntity("senemy");
		float angleOfEdge = (360.f / numberOfEdges) * numberOfEdge * (3.14159265359 / 180);
		float posOfEdgeX = (m_enemyConfig.SR * cos(angleOfEdge)) + bigEnemyTrans.pos.x;
		float posOfEdgeY = (m_enemyConfig.SR * sin(angleOfEdge)) + bigEnemyTrans.pos.y;
		// The speed has to be the same otherwise when the big enemy is destoryed it does not retain its orginial shape
		float speedOfEdgeX = m_enemyConfig.SMIN/4 * cos(angleOfEdge);
		float speedOfEdgeY = m_enemyConfig.SMIN/4 * sin(angleOfEdge);
		//std::cout << numberOfEdges << " " << numberOfEdge << " " << posOfEdgeX << " " << posOfEdgeY << " " << angleOfEdge << std::endl;
		smallEnemy->add<CTransform>(Vec2f(bigEnemyTrans.pos.x, bigEnemyTrans.pos.y), Vec2f(speedOfEdgeX, speedOfEdgeY), angleOfEdge);
		smallEnemy->add<CShape>(m_enemyConfig.SR / 2, numberOfEdges, bigEnemyShape.circle.getFillColor(), bigEnemyShape.circle.getOutlineColor(), bigEnemyShape.circle.getOutlineThickness());
		smallEnemy->add<CLifespan>(m_enemyConfig.L);
		smallEnemy->add<CScore>(numberOfEdges * 2 * 100);
	}
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
	// you should read the m_player->cInput component to determine if the player is moving
	auto &transform = player()->get<CTransform>();

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

	for (auto& entity : m_entities.getEntities()) {

		auto& entityMovement = entity->get<CTransform>();
		entityMovement.pos += entityMovement.velocity;
	}
}

void Game::sLifespan()
{
	for (auto& entity : m_entities.getEntities()) {
		if (entity->get<CLifespan>().exists) {
		auto& entityShape = entity->get<CShape>();
		auto& entityLifeSpan = entity->get<CLifespan>();
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
}

void Game::sCollision()
{
	// implement all proper collisions between entities
	// be sure to use the collision radius, NOT the shape radius
	for (auto& bullet : m_entities.getEntities("bullet")) {
		auto& bulletProp = bullet->get<CTransform>();
		//bulletProp.pos
		for (auto& enemy : m_entities.getEntities("enemy")) {
			// this is to ensure that this is only performed once if the enemy is active
			if (enemy.get()->isActive() && bullet.get()->isActive()) {
				auto& enemyProp = enemy->get<CTransform>();
				Vec2f vectorOfEntities = { (enemyProp.pos.x - bulletProp.pos.x), (enemyProp.pos.y - bulletProp.pos.y) };
				float distanceOfEntities = std::sqrt(vectorOfEntities.x * vectorOfEntities.x + vectorOfEntities.y * vectorOfEntities.y);
				if (bullet->get<CCollision>().radius + enemy->get<CCollision>().radius > distanceOfEntities) {
					enemy->destroy();
					bullet->destroy();
					spawnSmallEnemies(enemy);
					m_score += enemy->get<CScore>().score;
				}
			}
		}
		for (auto& smallEnemy : m_entities.getEntities("senemy")) {
			// this is to ensure that this is only performed once if the enemy is active
			if (smallEnemy.get()->isActive() && bullet.get()->isActive()) {
				auto& smallEnemyProp = smallEnemy->get<CTransform>();
				Vec2f vectorOfSmallEntities = { (smallEnemyProp.pos.x - bulletProp.pos.x), (smallEnemyProp.pos.y - bulletProp.pos.y) };
				float distanceOfSmallEntities = std::sqrt(vectorOfSmallEntities.x * vectorOfSmallEntities.x + vectorOfSmallEntities.y * vectorOfSmallEntities.y);
				if (bullet->get<CCollision>().radius + smallEnemy->get<CCollision>().radius > distanceOfSmallEntities) {
					smallEnemy->destroy();
					bullet->destroy();
					m_score += smallEnemy->get<CScore>().score;
				}
			}
		}
	}
	for (auto& player : m_entities.getEntities("player")) {
		for (auto& enemy : m_entities.getEntities("enemy")) {
			// this is to ensure that this is only performed once if the enemy is active
			if (enemy.get()->isActive()) {
				auto& enemyProp = enemy->get<CTransform>();
				Vec2f vectorOfEnemyandPlayer = { (enemyProp.pos.x - player->get<CTransform>().pos.x), (enemyProp.pos.y - player->get<CTransform>().pos.y) };
				float distanceOfEnemyandPlayer = std::sqrt(vectorOfEnemyandPlayer.x * vectorOfEnemyandPlayer.x + vectorOfEnemyandPlayer.y * vectorOfEnemyandPlayer.y);
				if (player->get<CCollision>().radius + enemy->get<CCollision>().radius > distanceOfEnemyandPlayer) {
					enemy->destroy();
					player->destroy();
					spawnSmallEnemies(enemy);
				}
			}
		}
		for (auto& smallEnemy : m_entities.getEntities("senemy")) {
			// this is to ensure that this is only performed once if the enemy is active
			if (smallEnemy.get()->isActive()) {
				auto& smallEnemyProp = smallEnemy->get<CTransform>();
				Vec2f vectorOfEnemyandPlayer = { (smallEnemyProp.pos.x - player->get<CTransform>().pos.x), (smallEnemyProp.pos.y - player->get<CTransform>().pos.y) };
				float distanceOfEnemyandPlayer = std::sqrt(vectorOfEnemyandPlayer.x * vectorOfEnemyandPlayer.x + vectorOfEnemyandPlayer.y * vectorOfEnemyandPlayer.y);
				if (player->get<CCollision>().radius + smallEnemy->get<CCollision>().radius > distanceOfEnemyandPlayer) {
					smallEnemy->destroy();
					player->destroy();
				}
			}
		}
	}
	// enemy wall collision
	for (auto& enemy : m_entities.getEntities("enemy")) {
		auto &enemyMovement = enemy->get<CTransform>();
		Vec2f checkMovement = enemyMovement.pos + enemyMovement.velocity;

		// Bounce off right and left window edges
		if (m_enemyConfig.SR + checkMovement.x > m_window.getSize().x || checkMovement.x - m_enemyConfig.SR  < 0) {
			enemyMovement.velocity.x = -enemyMovement.velocity.x;
		}
		// Bounce off bottom and top window edges
		if (m_enemyConfig.SR + checkMovement.y > m_window.getSize().y || checkMovement.y - m_enemyConfig.SR < 0) {
			enemyMovement.velocity.y = -enemyMovement.velocity.y;
		}
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

void Game::sEntityList(std::shared_ptr<Entity> entity)
{
	std::string entityLabel = std::to_string(entity->id());
	ImGui::BeginChild(entityLabel.c_str(), ImVec2(0, 40), false);
	ImGui::SameLine();
	float entityColorR = static_cast<float>(entity->get<CShape>().circle.getFillColor().r) / 255;
	float entityColorG = static_cast<float>(entity->get<CShape>().circle.getFillColor().g) / 255;
	float entityColorB = static_cast<float>(entity->get<CShape>().circle.getFillColor().b) / 255;
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(entityColorR, entityColorG, entityColorB, 0.5f));       // Background
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(entityColorR, entityColorG, entityColorB, 0.75f)); // On hover
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(entityColorR, entityColorG, entityColorB, 0.25f));  // On click
	if (ImGui::Button("D"))
	{
		entity->destroy();
	}
	ImGui::PopStyleColor(3); // Pop all 3 colors (active, hovered, base)
	ImGui::SameLine();
	ImGui::Text(entityLabel.c_str());
	ImGui::SameLine();
	ImGui::Text(entity->tag().c_str());
	ImGui::SameLine();
	auto& transform = entity->get<CTransform>();
	ImGui::Text(("(" + std::to_string(static_cast<int>(std::floorf(transform.pos.x))) + ", " + std::to_string(static_cast<int>(std::floorf(transform.pos.y))) + ")").c_str());
	ImGui::SameLine();
	ImGui::Text(("(" + std::to_string(static_cast<int>(std::floorf(transform.velocity.x))) + ", " + std::to_string(static_cast<int>(std::floorf(transform.velocity.x))) + ")").c_str());
	ImGui::EndChild();
}

void Game::sGUI()
{
	ImGui::Begin("Geometry Wars");
	if (ImGui::BeginTabBar("MainTabBar")) {
		// first tab menu
		if (ImGui::BeginTabItem("Systems")) {
			ImGui::Checkbox("Movement", &m_guiConfig.activeMovement);
			ImGui::Checkbox("Lifespan", &m_guiConfig.activeLifespan);
			ImGui::Checkbox("Collision", &m_guiConfig.activeCollision);
			ImGui::Checkbox("Spawning", &m_guiConfig.activeSpawning);
			ImGui::SliderInt("Spawn Time", &m_enemyConfig.SP, 1, 300);
			if (ImGui::Button("Manual Spawn"))
			{
				spawnEnemy();
			}
			ImGui::Checkbox("GUI", &m_guiConfig.activeGUI);
			ImGui::Checkbox("Rendering", &m_guiConfig.activeRendering);
			ImGui::EndTabItem();
		}
		// second tab menu
		if (ImGui::BeginTabItem("Entities")) {
			if (ImGui::CollapsingHeader("Entities")) {
				for (const auto& [tag, vec] : m_entities.getEntityMap()) {
					if (ImGui::CollapsingHeader(tag.c_str())) {
						for (const auto& entity : vec)
						{
							sEntityList(entity);
						}
					}
				}
			}
			if (ImGui::CollapsingHeader("All Entities")) {
				for (const auto& entity : m_entities.getEntities())
				{
					sEntityList(entity);
				}
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
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

	// To display text on the sfml window for amount of points
	m_text.setString("Points: " + std::to_string(m_score));
	m_window.draw(m_text);
	// draw the ui last
	ImGui::SFML::Render(m_window);

	m_window.display();
}

void Game::sUserInput()
{
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

		// this checks if the key is still pressed if it does not then it will return false
		auto& input = player()->add<CInput>();
		input.up = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
		input.left = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
		input.down = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
		input.right = sf::Keyboard::isKeyPressed(sf::Keyboard::D);

		// this event is triggered when a key is pressed
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P)
		{
			setPaused(!m_paused);
		}
		// to allow to bring back the GUI when it is switched off
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::G)
		{
			m_guiConfig.activeGUI = !m_guiConfig.activeGUI;
		}
		// pressing the esc key will close the game
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
		{
			m_running = false;
		}

		// update view if the player resize the window
		if (event.type == sf::Event::Resized) {
			sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
			m_window.setView(sf::View(visibleArea));
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
				//std::cout << "Left Mouse Button Clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")\n";
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
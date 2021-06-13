#include "MapLayer.h"

USING_NS_CC;

Layer* MapLayer::createScene(std::vector<Character*> gameHunter)
{
	return MapLayer::create(gameHunter);
}

// 0->map, 1->item/gun, 2->enemy, 3->bullet, 4->hunter
bool MapLayer::init(std::vector<Character*> gameHunter)
{
	if (!Layer::init())
	{
		return false;
	}
	this->scheduleUpdate();

	map = TMXTiledMap::create("map/Desert.tmx");
	addChild(map, 0);

	meta = map->getLayer("water");

	mapHeight = map->getMapSize().height;
	mapWidth = map->getMapSize().width;
	tileWidth = map->getTileSize().width;
	tileHeight = map->getTileSize().height;

	m_enemy = gameHunter;
	if (chclient != nullptr)
		hunter = m_enemy[chclient->getuid() - 1];
	else
		hunter = m_enemy[0];

	initSetItem();
	initBullet();
	if(chclient == nullptr)
		schedule(CC_SCHEDULE_SELECTOR(MapLayer::enemyFire), 0.5);

	AudioEngine::lazyInit();
	AudioEngine::preload("music/bulletEffect.wav");
	AudioEngine::preload("music/knifeEffect.wav");

	registerKeyboardEvent();
	registerTouchEvent();

	return true;
}

void MapLayer::initBullet() {
	for (auto& bullet : bullets) {
		bullet = Bullet::create("images/bullet.png");
		bullet->setVisible(false);
		bullet->setBulletActive(false);
		addChild(bullet, 3);
	}
}

//fix the animation problem
void MapLayer::registerKeyboardEvent() {
	auto listener = EventListenerKeyboard::create();

	listener->onKeyPressed = [=](EventKeyboard::KeyCode keyCode, Event* event) {
		//keyMap[keyCode] = true;
		switch (keyCode) {
		case EventKeyboard::KeyCode::KEY_D:
			hunter->m_speed[0] = true;
			if (hunter->m_speed[2] == false && hunter->m_speed[3] == false)
				hunter->runAction(hunter->getCharacterAnimRight());
			break;
		case EventKeyboard::KeyCode::KEY_A:
			hunter->m_speed[1] = true;
			if (hunter->m_speed[2] == false && hunter->m_speed[3] == false)
				hunter->runAction(hunter->getCharacterAnimLeft());
			break;
		case EventKeyboard::KeyCode::KEY_S:
			hunter->m_speed[2] = true;
			if (hunter->m_speed[0] == false && hunter->m_speed[1] == false)
				hunter->runAction(hunter->getCharacterAnimDown());
			break;
		case EventKeyboard::KeyCode::KEY_W:
			hunter->m_speed[3] = true;
			if (hunter->m_speed[0] == false && hunter->m_speed[1] == false)
				hunter->runAction(hunter->getCharacterAnimUp());
			break;
		case EventKeyboard::KeyCode::KEY_E:
			judgePick(hunter);
			break;
		default:
			break;
		}
	};

	listener->onKeyReleased = [=](EventKeyboard::KeyCode keyCode, Event* event) {
		hunter->stopAllActions();
		switch (keyCode) {
		case EventKeyboard::KeyCode::KEY_D:
			hunter->m_speed[0] = false;
			break;
		case EventKeyboard::KeyCode::KEY_A:
			hunter->m_speed[1] = false;
			break;
		case EventKeyboard::KeyCode::KEY_S:
			hunter->m_speed[2] = false;
			break;
		case EventKeyboard::KeyCode::KEY_W:
			hunter->m_speed[3] = false;
			break;
		default:
			break;
		}
		for (int i = 0; i <= 3; i++)
		{
			if (hunter->m_speed[i] == true)
			{
				switch (i) {
				case 0: hunter->runAction(hunter->getCharacterAnimRight());
					break;
				case 1: hunter->runAction(hunter->getCharacterAnimLeft());
					break;
				case 2: hunter->runAction(hunter->getCharacterAnimDown());
					break;
				case 3: hunter->runAction(hunter->getCharacterAnimUp());
					break;
				}
				break;
			}
		}
	};

	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void MapLayer::judgePick(Character* character) {
	Rect rect_character = character->getBoundingBox();
	for (auto weapon : weapons) {
		if (weapon->getBoundingBox().intersectsRect(rect_character)) {
			auto weaponType = weapon->getWeaponType();
			if (character->m_gun[weaponType] == nullptr) {
				weapon->retain();
				character->m_gun[weaponType] = weapon;
				character->setPlayerWeapon(weaponType);
				character->setPlayerRefresh(true);
				weapon->removeFromParent();
				weapons.erase(find(weapons.begin(), weapons.end(), weapon));
				break;
			}
		}
	}

	for (auto bn : m_bandage)
	{
		if (bn->getBoundingBox().intersectsRect(rect_character))
		{
			int bleed = character->getPlayerBleed() + bn->getRecoverHP();
			if (bleed > character->m_MAX_BLEED)
				bleed = character->m_MAX_BLEED;
			character->setPlayerBleed(bleed);
			bn->removeFromParent();
			m_bandage.erase(find(m_bandage.begin(), m_bandage.end(), bn));
			break;
		}
	}

	for (auto am : m_ammunition)
	{
		if (am->getBoundingBox().intersectsRect(rect_character))
		{
			character->setPlayerBullet(character->getPlayerBullet() + am->getAddBullet());
			am->removeFromParent();
			m_ammunition.erase(find(m_ammunition.begin(), m_ammunition.end(), am));
			break;
		}
	}
}

//add firing continuously
void MapLayer::registerTouchEvent() {
	auto touchListener = EventListenerTouchOneByOne::create();

	touchListener->onTouchBegan = [&](Touch* touch, Event* event) {
		auto weaponType = hunter->getPlayerWeapon();
		if (4 == weaponType) {
			auto knifeAudioID = AudioEngine::play2d("music/knifeEffect.mp3", false);
			AudioEngine::setVolume(knifeAudioID, *m_volume);
			makeKnifeAttack(hunter);

			return true;
		}

		hunter->bulletLocation = touch->getLocation();

		schedule(CC_SCHEDULE_SELECTOR(MapLayer::Fire), hunter->getBulletSpeed());
		return true;
	};

	touchListener->onTouchMoved = [&](Touch* touch, Event* event) {
		hunter->bulletLocation = touch->getLocation();
	};

	touchListener->onTouchEnded = [&](Touch* touch, Event* event) {
		unschedule(CC_SCHEDULE_SELECTOR(MapLayer::Fire));
		return true;
	};

	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
}

void MapLayer::makeKnifeAttack(Character* character) {
	Vec2 pos = character->getPosition();
	showEffect(pos);
	for (auto enemy : m_enemy) {
		if (enemy == character || enemy->getPlayerDeath())
			continue;
		Vec2 enemyPos = enemy->getPosition();
		if (enemyPos.getDistance(pos) < 100) {
			int bleed = enemy->getPlayerBleed() - 5 - character->getPlayerAttack();
			if (bleed < 0)
				bleed = 0;
			enemy->setPlayerBleed(bleed);
			showAttacked(enemy->getPosition());
		}
	}
}

void MapLayer::makeBulletAttack(Character* character, Weapon* weapon, float bulletX, float bulletY) {
	float time = sqrt(bulletX * bulletX + bulletY * bulletY) / 1000;
	float delta = 1;
	if (character == hunter)
		delta = 0.6;
	for (auto bullet : bullets) {
		if (!bullet->getBulletActive()) {
			bullet->setBulletActive(true);
			bullet->setPosition(character->getPositionX() + bulletX / time / 20, character->getPositionY() + bulletY / time / 20);
			bullet->setRotation(calRotation(bulletX, bulletY));
			bullet->runAction(RepeatForever::create(MoveBy::create(delta * weapon->getWeaponSpeed(), Vec2(bulletX / time, bulletY / time))));
			bullet->setBulletAttack(weapon->getWeaponAttack());
			bullet->setVisible(true);
			break;
		}
	}
}

void MapLayer::Fire(float dt)
{
	if (hunter->getPlayerBullet() > 0)
		hunter->setPlayerBullet(hunter->getPlayerBullet() - 1);
	else return;
	auto bulletAudioID = AudioEngine::play2d("music/bulletEffect.mp3", false);
	AudioEngine::setVolume(bulletAudioID, *m_volume);
	Vec2 bulletLocation = hunter->bulletLocation;
	Weapon* weapon = hunter->m_gun[hunter->getPlayerWeapon()];
	auto bulletX = bulletLocation.x - winSize.width / 2;
	auto bulletY = bulletLocation.y - winSize.height / 2;
	makeBulletAttack(hunter, weapon, bulletX, bulletY);
}

float MapLayer::calRotation(float bulletX, float bulletY) {
	if (bulletX == 0 && bulletY > 0)
		return -90.0f;
	else if (bulletX == 0 && bulletY < 0)
		return 90.0f;
	else if (bulletX > 0)
		return -180.0f * atan(bulletY / bulletX) / PI;
	else return -180.0f * atan(bulletY / bulletX) / PI + 180.0f;
}

void MapLayer::showEffect(Vec2 pos) {
	auto effectCircle = DrawNode::create();
	addChild(effectCircle, 2);
	effectCircle->drawSolidCircle(pos, 100.0f, CC_DEGREES_TO_RADIANS(360), 15, Color4F(0.28f, 0.46f, 1.0f, 0.6f));
	effectCircle->runAction(Sequence::create(FadeOut::create(0.5f), RemoveSelf::create(), NULL));
}

void MapLayer::showAttacked(Vec2 pos) {
	auto effectCircle = DrawNode::create();
	addChild(effectCircle, 5);
	effectCircle->drawSolidCircle(pos, 20.0f, CC_DEGREES_TO_RADIANS(360), 15, Color4F(1.0f, 0, 0, 0.6f));
	effectCircle->runAction(Sequence::create(FadeOut::create(0.3f), RemoveSelf::create(), NULL));
}

void MapLayer::update(float fDelta) {
	if (chclient != nullptr)
	{
		for (auto enemy : m_enemy) {
			if (enemy->getPlayerBleed() <= 0)
				enemy->setVisible(false);
		}
		for (auto bullet : bullets) {
			if (bullet->getBulletActive()) {
				auto bulletX = bullet->getPositionX();
				auto bulletY = bullet->getPositionY();
				if (bulletX < 0 || bulletX >= mapWidth * tileWidth || bulletY < 0 || bulletY >= mapHeight * tileHeight
					|| meta->getTileGIDAt(Vec2(bulletX / tileWidth, mapHeight - bulletY / tileHeight))) {
					bullet->setVisible(false);
					bullet->stopAllActions();
					bullet->setBulletActive(false);
					continue;
				}
				Rect rect_bullet = bullet->getBoundingBox();
				for (auto enemy : m_enemy) {
					if (enemy->getPlayerBleed() <= 0)
						continue;
					Rect rect_enemy = enemy->getBoundingBox();
					if (rect_enemy.intersectsRect(rect_bullet)) {
						showAttacked(enemy->getPosition());
						enemy->setPlayerBleed(enemy->getPlayerBleed() - bullet->getBulletAttack());
					}
				}
			}
		}
		if (chserver != nullptr)
		{
			chserver->map_update();
		}

		//本地的动作上传
		PlayerAction paction;
		paction.speed[0] = hunter->m_speed[0];
		paction.speed[1] = hunter->m_speed[1];
		paction.speed[2] = hunter->m_speed[2];
		paction.speed[3] = hunter->m_speed[3];
		chclient->upload(paction);

		//下载服务器端的数据并显示
		MapInformation& current_map = chclient->map;
		if (current_map.is_updated)
		{
			for (int i = 0; i < MAX_CONNECTIONS - 1; i++)
				if (current_map.player[i + 1].alive)
				{
					if (current_map.player[i + 1].position_x != save_map.player[i + 1].position_x || current_map.player[i + 1].position_y != save_map.player[i + 1].position_y)
					{
						m_enemy[i]->runAction(MoveTo::create(1.0f / 150.0f, Vec2(current_map.player[i + 1].position_x, current_map.player[i + 1].position_y)));
						CCLOG("MOVING: %f %f", current_map.player[i + 1].position_x, current_map.player[i + 1].position_y);
						
					}
				}
		}

		save_map = current_map;
	}
	else
	{
		for (auto enemy : m_enemy) {
			if (enemy->getPlayerBleed() <= 0)
				enemy->setVisible(false);
		}
		for (auto bullet : bullets) {
			if (bullet->getBulletActive()) {
				auto bulletX = bullet->getPositionX();
				auto bulletY = bullet->getPositionY();
				if (bulletX < 0 || bulletX >= mapWidth * tileWidth || bulletY < 0 || bulletY >= mapHeight * tileHeight
					|| meta->getTileGIDAt(Vec2(bulletX / tileWidth, mapHeight - bulletY / tileHeight))) {
					bullet->setVisible(false);
					bullet->stopAllActions();
					bullet->setBulletActive(false);
					continue;
				}
				Rect rect_bullet = bullet->getBoundingBox();
				for (auto enemy : m_enemy) {
					if (enemy->getPlayerBleed() <= 0)
						continue;
					Rect rect_enemy = enemy->getBoundingBox();
					if (rect_enemy.intersectsRect(rect_bullet)) {
						showAttacked(enemy->getPosition());
						enemy->setPlayerBleed(enemy->getPlayerBleed() - bullet->getBulletAttack());
					}
				}
			}
		}
		for (auto enemy : m_enemy)
		{
			if (enemy->getPlayerBleed() <= 0)
				continue;
			if (enemy != hunter) {
				judgePick(enemy);
				int nextT = enemy->getThought() + int(fDelta * 1000);
				enemy->setThought(nextT);
				if (nextT >= enemy->getThinkTime())
				{
					enemy->setThought(0);
					enemy->m_speed[0] = rand() % 2;
					enemy->m_speed[1] = rand() % 2;
					enemy->m_speed[2] = rand() % 2;
					enemy->m_speed[3] = rand() % 2;
					enemy->stopAllActions();
				}
			}
			float dx = 0, dy = 0;
			if (enemy->m_speed[0])
			{
				if (enemy != hunter && enemy->getThought() == 0)
					enemy->runAction(enemy->getCharacterAnimRight());
				dx = 4;
			}
			if (enemy->m_speed[1]) {
				if (enemy != hunter && enemy->getThought() == 0)
					enemy->runAction(enemy->getCharacterAnimLeft());
				dx = -4;
			}
			if (enemy->m_speed[2]) {
				if (enemy != hunter && enemy->getThought() == 0)
					enemy->runAction(enemy->getCharacterAnimDown());
				dy = -4;
			}
			if (enemy->m_speed[3]) {
				if (enemy != hunter && enemy->getThought() == 0)
					enemy->runAction(enemy->getCharacterAnimUp());
				dy = 4;
			}

			auto enemyPos = enemy->getPosition();
			auto nextX = enemyPos.x + dx;
			auto nextY = enemyPos.y + dy;

			auto nextMapX = nextX / tileWidth;
			auto nextMapY = mapHeight - nextY / tileHeight;

			if (nextMapX < mapWidth && nextMapX >= 0 && nextMapY < mapHeight && nextMapY >= 0
				&& !meta->getTileGIDAt(Vec2(nextMapX, nextMapY)))
				enemy->runAction(MoveTo::create(1.0f / 80.0f, Vec2(nextX, nextY)));
			else
				enemy->runAction(MoveTo::create(1.0f / 80.0f, Vec2(nextX - 2 * dx, nextY - 2 * dy)));
		}
		hunter->setPosition(hunter->getPosition());
	}
}


//set enemies/items randomly and at anywhere except water space.
template <class T>
void MapLayer::setRandPos(T* elem)
{
	float rx, ry, mrx, mry;
	while (true)
	{
		rx = random(50, static_cast<int>(mapWidth * tileWidth - 50 - 1));
		mrx = rx / tileWidth;
		ry = random(50, static_cast<int>(mapHeight * tileHeight - 50 - 1));
		mry = mapHeight - ry / tileWidth;
		if (mrx < mapWidth && mrx >= 0 && mry < mapHeight && mry >= 0 && !meta->getTileGIDAt(Vec2(mrx, mry)))
			break;
	}
	elem->setPosition(Vec2(rx, ry));
}

template <class T>
void MapLayer::initItem(std::vector<T*>& items, int number) {
	items.clear();
	for (int i = 0; i < number; i++)
	{
		items.push_back(T::create());
		addChild(items[i], 1);
		setRandPos(items[i]);
	}
}

void MapLayer::initSetItem()
{
	for (auto enemy : m_enemy) {
		addChild(enemy, 1);
		setRandPos(enemy);
	}

	hunter->setLocalZOrder(4);
	hunter->setPosition(Vec2(winSize.width / 2, winSize.height / 2));
	runAction(Follow::create(hunter, Rect::ZERO));

	initItem(weapons, m_weapon_number);
	initItem(m_bandage, m_bandage_number);
	initItem(m_ammunition, m_ammunition_number);

	Weapon* weapon = Weapon::create();
	weapon->retain();
	weapon->weaponInit(1, 1, 4, 0);
	hunter->m_gun[4] = weapon;

	hunter->setPlayerRefresh(true);
}

//aim at hunter automatically and fire
void MapLayer::enemyFire(float delt)
{
	Rect rect_hunter = hunter->getBoundingBox();
	for (auto enemy : m_enemy)
	{
		if (enemy->getPlayerDeath() || enemy == hunter)
			continue;
		Rect rect_enemy(enemy->getPosition().x - 300, enemy->getPosition().y - 300, 600, 600);
		if (rect_enemy.intersectsRect(rect_hunter))
		{
			auto weaponType = enemy->getPlayerWeapon();
			if (4 == weaponType)
			{
				makeKnifeAttack(enemy);
				continue;
			}
			if (enemy->getPlayerBullet() > 0)
				enemy->setPlayerBullet(enemy->getPlayerBullet() - 1);
			else continue;
			Weapon* weapon = enemy->m_gun[weaponType];
			auto bulletLocation = hunter->getPosition();    //enemy aims at hunter
			auto bulletX = bulletLocation.x - enemy->getPosition().x;
			auto bulletY = bulletLocation.y - enemy->getPosition().y;
			makeBulletAttack(enemy, weapon, bulletX, bulletY);
		}
	}
}
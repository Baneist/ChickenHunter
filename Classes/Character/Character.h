#ifndef _CHARACTER_H_
#define _CHARACTER_H_

#include "cocos2d.h"
#include "Character/Character.h"
#include <vector>

USING_NS_CC;

class Character : public Sprite {
public:
	Character();

	const int m_MAX_BLEED = 100;

	static cocos2d::Sprite* createSprite();
	virtual bool init();
	CREATE_FUNC(Character);

	CC_SYNTHESIZE(Animate*, m_character_anim_down, CharacterAnimDown);
	CC_SYNTHESIZE(Animate*, m_character_anim_left, CharacterAnimLeft);
	CC_SYNTHESIZE(Animate*, m_character_anim_right, CharacterAnimRight);
	CC_SYNTHESIZE(Animate*, m_character_anim_up, CharacterAnimUp);
	CC_SYNTHESIZE(SpriteFrameCache*, m_character_frame_cache, FrameCache);

	void initName();
	void initSprite();
	void initAnimate();

private:

	CC_SYNTHESIZE(std::string, m_name, PlayerName);
	CC_SYNTHESIZE(int, m_bleed, PlayerBleed);
	CC_SYNTHESIZE(int, m_attack, PlayerAttack);
	CC_SYNTHESIZE(int, m_defense, PlayerDefense);

protected:

	int m_bandage[2];
	int m_gun[4];

public: 
	int* getPlayerBandage();
	void setPlayerBandage(int var, int pos);

	int* getPlayerGun();
	void setPlayerGun(int var, int pos);

};


#endif // !_CHARACTER_H_
#pragma once
#include <Prefabs/GameMenuPrefab.h>

class GameMenu : public GameScene
{
public:
	GameMenu() : GameScene(L"GameMenu") {};
	~GameMenu() override = default;
	GameMenu(const GameMenu& other) = delete;
	GameMenu(GameMenu&& other) noexcept = delete;
	GameMenu& operator=(const GameMenu& other) = delete;
	GameMenu& operator=(GameMenu&& other) noexcept = delete;
	void SetActiveMenu(MenuTypes menuType);
	GameObject* m_pControls{ nullptr };
	GameMenuPrefab* m_pMenu{ nullptr };

protected:
	void Initialize() override;

private:
	enum MenuInputIds
	{
		Select = 0,
		MenuUp = 1,
		MenuDown = 2,
		Exit = 3,
		Controls = 4
	};


};


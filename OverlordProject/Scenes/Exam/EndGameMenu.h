#pragma once
#include <Prefabs/GameMenuPrefab.h>

class EndGameMenu : public GameScene
{
public:
	EndGameMenu() : GameScene(L"EndGameMenu") {};
	~EndGameMenu() override = default;
	EndGameMenu(const EndGameMenu& other) = delete;
	EndGameMenu(EndGameMenu&& other) noexcept = delete;
	EndGameMenu& operator=(const EndGameMenu& other) = delete;
	EndGameMenu& operator=(EndGameMenu&& other) noexcept = delete;
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
		Controls = 4,
	};

};


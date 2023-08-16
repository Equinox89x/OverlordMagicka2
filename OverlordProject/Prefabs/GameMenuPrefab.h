#pragma once
enum class InGameMenuItems {
	ToMenu = 0,
	Restart = 1,
	Exit = 2
};

enum class MenuItems {
	Start = 0,
	Exit = 1
};

enum class MenuTypes {
	Menu,
	InGameMenu,
	EndMenu
};

struct MenuDesc
{
	MenuDesc(){}

	int actionId_Select{ -1 };
	int actionId_MenuUp{ -1 };
	int actionId_MenuDown{ -1 };
	int actionId_Exit{ -1 };
	int actionId_Controls{ -1 };

};

class GameMenuPrefab : public GameObject
{
public:
	GameMenuPrefab(const MenuDesc& menuDesc, MenuTypes menuType);
	~GameMenuPrefab() override = default;

	GameMenuPrefab(const GameMenuPrefab& other) = delete;
	GameMenuPrefab(GameMenuPrefab&& other) noexcept = delete;
	GameMenuPrefab& operator=(const GameMenuPrefab& other) = delete;
	GameMenuPrefab& operator=(GameMenuPrefab&& other) noexcept = delete;

	void DrawImGui();
	void SetActiveMenu(MenuTypes menuItem);

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:
	FMOD::Channel* CurrentChannel;
	FMOD::Sound* GameMusic;
	FMOD::Sound* MenuMusic;
	FMOD::System* SoundSystem;


	MenuDesc m_MenuDesc;
	float ButtonTimer{ 0.2f };
	bool CanButton{ true };
	MenuTypes MenuType;
	MenuItems SelectedItem{ MenuItems::Start };
	InGameMenuItems SelectedInGameItem{ InGameMenuItems::ToMenu };

	//template<typename MenuType>
	//MenuType GetSelectedMenuItem();
	//template<typename MenuType>
	//void CycleSelectedMenuItem(bool isDown);



template<typename MenuType>
MenuType* GetSelectedMenuItem() {
    if constexpr (std::is_same_v<MenuType, MenuItems>) {
        return static_cast<MenuType*>(&SelectedItem);
    }

    if constexpr (std::is_same_v<MenuType, InGameMenuItems>) {
        return static_cast<MenuType*>(&SelectedInGameItem);
    }

    //return nullptr;
}

template<typename MenuType>
void CycleSelectedMenuItem(bool isDown)
{
	if constexpr (std::is_same_v<MenuType, InGameMenuItems>) {
		if (!isDown) {
			SelectedInGameItem == InGameMenuItems::ToMenu ?
				SelectedInGameItem = InGameMenuItems(0) :
				SelectedInGameItem = InGameMenuItems(static_cast<int>(SelectedInGameItem) - 1);
		}
		else {
			SelectedInGameItem == InGameMenuItems::Exit ?
				SelectedInGameItem = InGameMenuItems(2) :
				SelectedInGameItem = InGameMenuItems(static_cast<int>(SelectedInGameItem) + 1);
		}

		switch (SelectedInGameItem)
		{
		case InGameMenuItems::ToMenu:
			GetComponent<SpriteComponent>()->SetTexture(L"Textures/Magicka/Game_ToMenu.png");
			break;
		case InGameMenuItems::Restart:
			GetComponent<SpriteComponent>()->SetTexture(L"Textures/Magicka/Game_Restart.png");
			break;
		case InGameMenuItems::Exit:
			GetComponent<SpriteComponent>()->SetTexture(L"Textures/Magicka/Game_MenuExit.png");
			break;
		default:
			break;
		}
	}

	if constexpr (std::is_same_v<MenuType, MenuItems>) {
		if (!isDown) {
			SelectedItem == MenuItems::Start ?
				SelectedItem = MenuItems(0) :
				SelectedItem = MenuItems(static_cast<int>(SelectedItem) - 1);
		}
		else {
			SelectedItem == MenuItems::Exit ?
				SelectedItem = MenuItems(1) :
				SelectedItem = MenuItems(static_cast<int>(SelectedItem) + 1);
		}

		switch (SelectedItem)
		{
		case MenuItems::Start:
			GetComponent<SpriteComponent>()->SetTexture(L"Textures/Magicka/Menu_Start.png");
			break;
		case MenuItems::Exit:
			GetComponent<SpriteComponent>()->SetTexture(L"Textures/Magicka/Menu_Exit.png");
			break;
		default:
			break;
		}
	}
}
};
#include "stdafx.h"
#include "GameMenuPrefab.h"
#include <Scenes/Exam/Menu.h>
#include <Scenes/Exam/EndGameMenu.h>
#include <Scenes/Exam/GameMenu.h>

GameMenuPrefab::GameMenuPrefab(const MenuDesc& menuDesc, MenuTypes menuType) :
	m_MenuDesc{ menuDesc },
	MenuType{ menuType }
{
}

void GameMenuPrefab::DrawImGui()
{
}

void GameMenuPrefab::Initialize(const SceneContext&)
{
	switch (MenuType)
	{
	case MenuTypes::Menu:
		AddComponent(new SpriteComponent(L"Textures/Magicka/Menu_Start.png"));
		break;
	case MenuTypes::InGameMenu:
		AddComponent(new SpriteComponent(L"Textures/Magicka/Game_ToMenu.png"));
		break;
	case MenuTypes::EndMenu:
		AddComponent(new SpriteComponent(L"Textures/Magicka/Game_Won.png"));
		break;
	default:
		break;
	}

}

void GameMenuPrefab::SetActiveMenu(MenuTypes menuItem)
{
	switch (menuItem)
	{
	case MenuTypes::Menu:
		GetComponent<SpriteComponent>()->SetTexture(L"Textures/Magicka/Menu_Start.png");
		break;
	case MenuTypes::InGameMenu:
		GetComponent<SpriteComponent>()->SetTexture(L"Textures/Magicka/Game_ToMenu.png");
		break;
	case MenuTypes::EndMenu:
		GetComponent<SpriteComponent>()->SetTexture(L"Textures/Magicka/Game_Won.png");
		break;
	default:
		break;
	}

}

void GameMenuPrefab::Update(const SceneContext& sceneContext)
{
	float deltaTime = sceneContext.pGameTime->GetElapsed();

	if (!CanButton) {
		ButtonTimer -= deltaTime;
		if (ButtonTimer <= 0) {
			ButtonTimer = 0.2f;
			CanButton = true;
		}
	}

	if (MenuType == MenuTypes::EndMenu) {
		if (SceneManager::Get()->HasWon) {
			GetComponent<SpriteComponent>()->SetTexture(L"Textures/Magicka/Game_Won.png");
		}
		else {
			GetComponent<SpriteComponent>()->SetTexture(L"Textures/Magicka/Game_Lost.png");
		}
	}

	if (sceneContext.pInput->IsActionTriggered(m_MenuDesc.actionId_Controls)) {
		if (CanButton) {
			if (MenuType == MenuTypes::Menu) {
				Menu* menu1{ dynamic_cast<Menu*>(SceneManager::Get()->GetActiveScene()) };
				menu1->m_pControls->SetHidden(!menu1->m_pControls->GetHidden());
				menu1->m_pMenu->SetHidden(!menu1->m_pMenu->GetHidden());
			}
			else if (MenuType == MenuTypes::InGameMenu) {
				GameMenu* menu2{ dynamic_cast<GameMenu*>(SceneManager::Get()->GetActiveScene()) };
				menu2->m_pControls->SetHidden(!menu2->m_pControls->GetHidden());
				menu2->m_pMenu->SetHidden(!menu2->m_pMenu->GetHidden());
			}
			else if (MenuType == MenuTypes::EndMenu){
				EndGameMenu* menu3{ dynamic_cast<EndGameMenu*>(SceneManager::Get()->GetActiveScene()) };
				menu3->m_pControls->SetHidden(!menu3->m_pControls->GetHidden());
				menu3->m_pMenu->SetHidden(!menu3->m_pMenu->GetHidden());
			}
			CanButton = false;
		}
	}

	
	//Menu* scene{ reinterpret_cast<Menu*>(SceneManager::Get()->GetActiveScene()) };
	if (sceneContext.pInput->IsActionTriggered(m_MenuDesc.actionId_Select)) {
		if (CanButton) {
			if (MenuType == MenuTypes::Menu) {

				switch (*GetSelectedMenuItem<MenuItems>())
				{
				case MenuItems::Start:
					SceneManager::Get()->SetActiveGameScene(L"MagickaScene");
					break;
				case MenuItems::Exit:
					PostQuitMessage(0);
					break;
				default:
					break;
				}
			}
			else if (MenuType == MenuTypes::InGameMenu) {
				switch (*GetSelectedMenuItem<InGameMenuItems>())
				{
				case InGameMenuItems::ToMenu:
					SceneManager::Get()->SetActiveGameScene(L"Menu");
					break;
				case InGameMenuItems::Restart:
					SceneManager::Get()->RestartTriggered = true;
					SceneManager::Get()->SetActiveGameScene(L"MagickaScene");
					break;
				case InGameMenuItems::Exit:
					PostQuitMessage(0);
					break;
				default:
					break;
				}
			}
			else {
				SceneManager::Get()->SetActiveGameScene(L"Menu");
			}
			CanButton = false;
		}
	}

	if (sceneContext.pInput->IsActionTriggered(m_MenuDesc.actionId_MenuUp)) {
		if (CanButton) {
			switch (MenuType)
			{
			case MenuTypes::Menu:
				CycleSelectedMenuItem<MenuItems>(false);
				break;
			case MenuTypes::InGameMenu:
				CycleSelectedMenuItem<InGameMenuItems>(false);
				break;
			case MenuTypes::EndMenu:
				break;
			default:
				break;
			}
			CanButton = false;
		}
	}
	if (sceneContext.pInput->IsActionTriggered(m_MenuDesc.actionId_MenuDown)) {
		if (CanButton) {
			switch (MenuType)
			{
			case MenuTypes::Menu:
				CycleSelectedMenuItem<MenuItems>(true);
				break;
			case MenuTypes::InGameMenu:
				CycleSelectedMenuItem<InGameMenuItems>(true);
				break;
			case MenuTypes::EndMenu:
				break;
			default:
				break;
			}
			CanButton = false;
		}
	}

	if (sceneContext.pInput->IsActionTriggered(m_MenuDesc.actionId_Exit)) {
		if (CanButton) {
			SceneManager::Get()->SetActiveGameScene(L"MagickaScene");
			CanButton = false;
		}
	}

}
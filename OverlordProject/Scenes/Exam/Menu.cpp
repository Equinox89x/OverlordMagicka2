#include "stdafx.h"
#include "Menu.h"

void Menu::Initialize()
{

	MenuDesc menuDesc{ };
	menuDesc.actionId_MenuDown = MenuDown;
	menuDesc.actionId_MenuUp = MenuUp;
	menuDesc.actionId_Select = Select;
	menuDesc.actionId_Exit = Exit;
	menuDesc.actionId_Controls = Controls;

	auto go{ new GameMenuPrefab(menuDesc, MenuTypes::Menu) };
	m_pMenu = AddChild(go);	

	auto go2{ new GameObject() };
	m_pControls = AddChild(go2);
	m_pControls->AddComponent(new SpriteComponent(L"Textures/controls.png"));
	m_pControls->SetHidden(true);

	auto inputAction = InputAction(Select, InputState::down, VK_SPACE, -1, XINPUT_GAMEPAD_X);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(MenuUp, InputState::down, VK_UP, -1, XINPUT_GAMEPAD_DPAD_UP);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(MenuDown, InputState::down, VK_DOWN, -1, XINPUT_GAMEPAD_DPAD_DOWN);
	m_SceneContext.pInput->AddInputAction(inputAction);	
	
	inputAction = InputAction(Exit, InputState::down, VK_ESCAPE, -1, XINPUT_GAMEPAD_START);
	m_SceneContext.pInput->AddInputAction(inputAction);	
	
	inputAction = InputAction(Controls, InputState::down, VK_TAB, -1);
	m_SceneContext.pInput->AddInputAction(inputAction);
}

void Menu::SetActiveMenu(MenuTypes menuType)
{
	m_pMenu->SetActiveMenu(menuType);
}
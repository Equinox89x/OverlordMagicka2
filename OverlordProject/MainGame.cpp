#include "stdafx.h"
#include "MainGame.h"

#include "Scenes/Exam/Menu.h"
#include "Scenes/Exam/EndGameMenu.h"
#include "Scenes/Exam/GameMenu.h"
#include "Scenes/Exam/MagickaScene.h"

//Game is preparing
void MainGame::OnGamePreparing(GameContext& gameContext)
{
	gameContext.windowTitle = L"GP2 - Magicka 2 (2023) | (2DAE13) Bram Robyn";
}

void MainGame::Initialize()
{
	SceneManager::Get()->AddGameScene(new Menu());
	SceneManager::Get()->AddGameScene(new EndGameMenu());
	SceneManager::Get()->AddGameScene(new GameMenu());
	SceneManager::Get()->AddGameScene(new MagickaScene());
}

LRESULT MainGame::WindowProcedureHook(HWND /*hWnd*/, UINT message, WPARAM wParam, LPARAM lParam)
{

	if(message == WM_KEYUP)
	{
		if ((lParam & 0x80000000) != 0x80000000)
			return -1;

		//[F1] Toggle Scene Info Overlay
		if(wParam == VK_F1)
		{
			const auto pScene = SceneManager::Get()->GetActiveScene();
			pScene->GetSceneSettings().Toggle_ShowInfoOverlay();
		}

		//[F2] Toggle Debug Renderer (Global)
		if (wParam == VK_F2)
		{
			DebugRenderer::ToggleDebugRenderer();
			return 0;

		}

		//[F3] Previous Scene
		if (wParam == VK_F3)
		{
			SceneManager::Get()->PreviousScene();
			return 0;

		}

		//[F4] Next Scene
		if (wParam == VK_F4)
		{
			SceneManager::Get()->NextScene();
			return 0;
		}

		//[F5] If PhysX Framestepping is enables > Next Frame	
		if (wParam == VK_F6)
		{
			const auto pScene = SceneManager::Get()->GetActiveScene();
			pScene->GetPhysxProxy()->NextPhysXFrame();
		}
	}
	

	return -1;
}

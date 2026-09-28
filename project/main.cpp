#include "Game.h"

//Windouwsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) 
{
	CoInitializeEx(0, COINIT_MULTITHREADED);

	Game game;

	game.Initialiaze();
	
	//ウィンドウの×ボタンが押されるまでループ
	while (true) 
	{
		game.Update();
		if (game.IsEndRequst())
		{
			break;
		}

		game.Draw();
	}
	game.Finalize();

	return 0;
}

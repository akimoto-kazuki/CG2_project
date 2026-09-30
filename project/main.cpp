#include "Game.h"

//Windouwsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) 
{
	CoInitializeEx(0, COINIT_MULTITHREADED);

	Framework* game = new Game();

	game->Run();

	delete game;

	return 0;
}

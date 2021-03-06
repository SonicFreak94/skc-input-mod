#include "stdafx.h"
#include "GamePad.h"
#include <FileSystem.h>

VoidFunc(Poll_JoysticksAndKeyboard, 0x0040BCDB);
FunctionPointer(int, Poll_Controller1, (void), 0x0040C3D2);
FunctionPointer(int, Poll_Controller2, (void), 0x0040C4D0);

DataPointer(int32_t, dword_82DDCC, 0x82DDCC);

GamePad game_pads[2] {};

void poll_sdl()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			default:
				break;

			case SDL_JOYDEVICEADDED:
			{
				const int which = event.cdevice.which;

				for (auto& controller : game_pads)
				{
					// Checking for both in cases like the DualShock 4 and (e.g.) DS4Windows where the controller might be
					// "connected" twice with the same ID. GamePad::open automatically closes if already open.
					if (!controller.connected() || controller.controller_id() == which)
					{
						controller.open(which);
						break;
					}
				}

				break;
			}

			case SDL_JOYDEVICEREMOVED:
			{
				const int which = event.cdevice.which;

				for (auto& controller : game_pads)
				{
					if (controller.controller_id() == which)
					{
						controller.close();
						break;
					}
				}

				break;
			}
		}
	}

	SDL_GameControllerUpdate();
}

static int __cdecl Poll_Controller1_r()
{
	if (dword_82DDCC >= 0)
	{
		return Poll_Controller1();
	}

	return Poll_Controller1() | (game_pads[0].pressed() | game_pads[0].held() << 8);
}

static int __cdecl Poll_Controller2_r()
{
	if (!dword_82DDCC)
	{
		return Poll_Controller2();
	}

	return Poll_Controller2() | (game_pads[1].pressed() | game_pads[1].held() << 8);
}

static void __cdecl poll_controllers()
{
	poll_sdl();

	game_pads[0].poll();
	game_pads[1].poll();

	Poll_JoysticksAndKeyboard();
}

extern "C"
{
	__declspec(dllexport) ModInfo SKCModInfo = { ModLoaderVer };

	__declspec(dllexport) void Init(const wchar_t* path)
	{
		std::wstring dll = path;
		dll.append(L"\\SDL2.dll");

		auto handle = LoadLibraryW(dll.c_str());

		if (handle == nullptr)
		{
			printf("[Input] Failed to load SDL2 DLL.\n");
			return;
		}

		int init;
		if ((init = SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS)) != 0)
		{
			printf("[Input] Unable to initialize SDL. Error code: %i\n", init);
			MessageBoxA(nullptr, "Error initializing SDL. See debug message for details.",
						"SDL Init Error", MB_OK | MB_ICONERROR);
			return;
		}

		std::wstring dbpath = path;
		dbpath.append(L"\\gamecontrollerdb.txt");

		if (FileExists(dbpath))
		{
			std::string dbpath_a(dbpath.begin(), dbpath.end());

			int result = SDL_GameControllerAddMappingsFromFile(dbpath_a.c_str());

			if (result == -1)
			{
				printf("[Input] Error loading gamecontrollerdb: %s\n", SDL_GetError());
			}
			else
			{
				printf("[Input] Controller mappings loaded: %i\n", result);
			}
		}

		WriteJump(reinterpret_cast<void*>(0x0040BD52), reinterpret_cast<void*>(0x0040BEC6));
		WriteJump(reinterpret_cast<void*>(0x0040BEDA), reinterpret_cast<void*>(0x0040C04F));
		WriteCall(reinterpret_cast<void*>(0x00405A4E), &poll_controllers);
		WriteCall(reinterpret_cast<void*>(0x00405A53), &Poll_Controller1_r);
		WriteCall(reinterpret_cast<void*>(0x00405A5C), &Poll_Controller2_r);
	}
}

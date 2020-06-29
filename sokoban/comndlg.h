#include <Windows.h>

const char Filter[] = { "MAP DB (*.txt)\0*.txt\0" };

BOOL OpenLoad(HWND hWnd, LPSTR Path, DWORD PathSize)
{
	OPENFILENAME ofn = { 0 };

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = Filter;
	ofn.lpstrFile = Path;
	ofn.nMaxFile = 256;
	ofn.lpstrInitialDir = ".";

	if (GetOpenFileName(&ofn))
	{
		memcpy(Path, ofn.lpstrFile, PathSize);
		return 1;
	}

	return 0;
}

BOOL OpenSave(HWND hWnd, LPSTR Path, DWORD PathSize)
{
	OPENFILENAME sfn = { 0 };

	sfn.lStructSize = sizeof(OPENFILENAME);
	sfn.hwndOwner = hWnd;
	sfn.lpstrFilter = Filter;
	sfn.lpstrFile = Path;
	sfn.nMaxFile = 256;
	sfn.lpstrInitialDir = ".";

	if (GetSaveFileName(&sfn))
	{
		memcpy(Path, sfn.lpstrFile, PathSize);
		return 1;
	}

	return 0;
}
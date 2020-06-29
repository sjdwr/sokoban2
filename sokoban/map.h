#include <Windows.h>
#include <stdio.h>

#define WALL 1
#define BOTTOM 2
#define BOX 4
#define GOAL 8
#define PLAYER 16

#define TOOL_WIDTH 70
#define TOOL_HEIGHT 352

#define BITMAP_SIZE 32

typedef struct 
{
	UINT Stage;
	UINT MAP_WIDTH;
	UINT MAP_HEIGHT;
	UINT PlayerX;
	UINT PlayerY;
	UINT TotalMapCount;
	BYTE *MAP;
}MapHdr;

typedef enum
{
	Replacement = 0,
	Addition,
	NewMap
}SaveMapType;

BOOL IsHuman(MapHdr*);

BOOL ReadMap(LPCSTR PATH, UINT Stage, MapHdr *_MAPHDR)
{
	FILE *fp = fopen(PATH, "r");
	char str[1000];
	UINT i, j, End;
	BYTE Type, MapSt;

	if (fp)
	{
		memset(str, 0, sizeof(str));

		while (!feof(fp))
		{
			fread(str, sizeof(BYTE), 1, fp);

			if (*str == ':')
			{
				fread(str, sizeof(BYTE), 5, fp);

				if (!strcmp(str, "MAPCT"))
				{
					fscanf(fp, ",%d", &_MAPHDR->TotalMapCount);
					continue;
				}
				else if (!strcmp(str, "STAGE"))
				{
					fscanf(fp, ",%d,%d,%d", &_MAPHDR->Stage, &_MAPHDR->MAP_WIDTH, &_MAPHDR->MAP_HEIGHT);

					if (Stage != _MAPHDR->Stage)
						continue;

					_MAPHDR->MAP = (BYTE*)malloc(sizeof(BYTE) * _MAPHDR->MAP_WIDTH * _MAPHDR->MAP_HEIGHT);
					fread(str, sizeof(BYTE), sizeof(str), fp);

					i = j = MapSt = 0;
					Type = str[i];

					while (Type)
					{
						if (Type != 13 && Type != 10)
						{
							if (Type == '}' && MapSt)
							{
								fclose(fp);
								return 1;
							}

							if (MapSt)
							{
								End = i;
								while (str[End] != ',' && str[End] != 0) ++End;
								str[End] = 0;

								_MAPHDR->MAP[j++] = atoi(str + i);
								i = End;


							}

							if (Type == '{')
								MapSt = 1;
						}
						++i;
						Type = str[i];
					}
				}
			}
		}

		fclose(fp);
		return 0;
	}

	return 0;
}


BOOL SaveMap(LPCSTR PATH, UINT Stage, SaveMapType SaveType, MapHdr *_MAPHDR)
{
	char str[100] = { 0 };
	BYTE Head[20] = { 0 };

	FILE *fp_r;
	FILE *fp_w;

	UINT i, j, k;

	if (SaveType == 0) // 내용교체
	{
		fp_r = fopen(PATH, "r");

		if (fp_r)
		{
			strcpy(str, PATH);
			strcat(str, "__");

			fp_w = fopen(str, "w");

			if (!fp_w)
			{
				fclose(fp_r);
				return 0;
			}

			memset(str, 0, sizeof(str));

			while (!feof(fp_r))
			{
				if (!fread_s(str, sizeof(BYTE), 1, 1, fp_r))
					break;

				if (*str == ':')
				{
					fread(str, sizeof(BYTE), 5, fp_r);

					if (!strcmp(str, "MAPCT"))
					{
						fscanf(fp_r, ",%d", &i);
						fprintf(fp_w, ":MAPCT,%d", _MAPHDR->TotalMapCount);
					}
					else if (!strcmp(str, "STAGE"))
					{
						fscanf(fp_r, ",%d,%d,%d", &i, &j, &k);

						if (i == Stage)
						{
							fprintf(fp_w, ":STAGE,%d,%d,%d\n{\n", Stage, _MAPHDR->MAP_WIDTH, _MAPHDR->MAP_HEIGHT);

							for (i = 0, j = 0; i < _MAPHDR->MAP_HEIGHT * _MAPHDR->MAP_WIDTH; ++i, ++j)
							{
								if (j >= _MAPHDR->MAP_WIDTH)
								{
									j = 0;
									fprintf(fp_w, "\n");
								}

								fprintf(fp_w, "%d,", _MAPHDR->MAP[i]);
							}

							fprintf(fp_w, "\n}");

							while (*str != '}')
								fread(str, sizeof(BYTE), 1, fp_r);
						}
						else
							fprintf(fp_w, ":STAGE,%d,%d,%d", i, j, k);

					}
				}
				else
					fwrite(str, sizeof(BYTE), 1, fp_w);
			}

			fclose(fp_r);
			fclose(fp_w);

			remove(PATH);

			strcpy(str, PATH);
			strcat(str, "__");

			rename(str, PATH);
		}
	}
	else if (SaveType == 1) // 뒤에 붙이기
	{
		fp_w = fopen(PATH, "r+");

		if (fp_w)
		{
			while (!feof(fp_w))
			{
				if (!fread_s(str, sizeof(BYTE), 1, 1, fp_w))
					break;

				if (*str == ':')
				{
					fread(str, sizeof(BYTE), 5, fp_w);

					if (!strcmp(str, "MAPCT"))
					{
						fseek(fp_w, ftell(fp_w) - 6, SEEK_SET);
						fprintf(fp_w, ":MAPCT,%d", Stage);
						break;
					}
				}
			}

			fclose(fp_w);
		}

		fp_w = fopen(PATH, "a+");

		if (fp_w)
		{
			fprintf(fp_w, "\n:STAGE,%d,%d,%d\n{\n", _MAPHDR->Stage, _MAPHDR->MAP_WIDTH, _MAPHDR->MAP_HEIGHT);

			for (i = 0, j = 0; i < _MAPHDR->MAP_HEIGHT * _MAPHDR->MAP_WIDTH; ++i, ++j)
			{
				if (j >= _MAPHDR->MAP_WIDTH)
				{
					j = 0;
					fprintf(fp_w, "\n");
				}

				fprintf(fp_w, "%d,", _MAPHDR->MAP[i]);
			}

			fprintf(fp_w, "\n}");

			fclose(fp_w);

			return 0;
		}
	}
	else if (SaveType == 2) // 새로운 맵
	{
		fp_w = fopen(PATH, "w");

		if (!fp_w)
			return 0;

		fprintf(fp_w, ":MAPCT,1\n:STAGE,%d,%d,%d\n{\n", _MAPHDR->Stage, _MAPHDR->MAP_WIDTH, _MAPHDR->MAP_HEIGHT);

		for (i = 0, j = 0; i < _MAPHDR->MAP_HEIGHT * _MAPHDR->MAP_WIDTH; ++i, ++j)
		{
			if (j >= _MAPHDR->MAP_WIDTH)
			{
				j = 0;
				fprintf(fp_w, "\n");
			}

			fprintf(fp_w, "%d,", _MAPHDR->MAP[i]);
		}

		fprintf(fp_w, "\n}");

		fclose(fp_w);
	}
 
	return 0;
}

VOID EditMap(UINT x, UINT y, int Select, MapHdr *_MapHdr)
{
	UINT Index;

	Index = (x - 70) / 32 + (y / 32) * _MapHdr->MAP_WIDTH;

	switch (Select)
	{
		case 0:
			_MapHdr->MAP[Index] = BOTTOM;
			break;
		case 1:
			_MapHdr->MAP[Index] = BOTTOM | BOX;
			break;
		case 2:
			_MapHdr->MAP[Index] = GOAL | BOX;
			break;
		case 3:
			_MapHdr->MAP[Index] = WALL;
			break;
		case 4:
			_MapHdr->MAP[Index] = 0;
			break;
		case 5:
			_MapHdr->MAP[Index] = GOAL;
			break;
		case 6:
		{
			if (IsHuman(_MapHdr)) return;

			_MapHdr->MAP[Index] = PLAYER | BOTTOM;
			break;
		}
	}
}

VOID CreateMap(UINT DrawX, HDC hDC, HBITMAP *hBitmapArray, UINT MaxRes, MapHdr *_MapHdr, UINT EditMode)
{
	HDC hMemDC, hMemDC2;
	HBITMAP hMapPapaer;
	UINT i, j, Index, Bitmap_IDX = 0, chkGoal = 0;

	UINT BMP_MAP_WIDTH = _MapHdr->MAP_WIDTH * BITMAP_SIZE;
	UINT BMP_MAP_HEIGHT = _MapHdr->MAP_HEIGHT * BITMAP_SIZE;

	hMapPapaer = CreateCompatibleBitmap(hDC, BMP_MAP_WIDTH, BMP_MAP_HEIGHT);

	hMemDC = CreateCompatibleDC(hDC);
	hMemDC2 = CreateCompatibleDC(hMemDC);

	SelectObject(hMemDC, hMapPapaer);

	for (j = 0; j < _MapHdr->MAP_HEIGHT; ++j)
	{
		for (i = 0; i < _MapHdr->MAP_WIDTH; ++i)
		{
			Index = j * _MapHdr->MAP_WIDTH + i;

			switch (_MapHdr->MAP[Index])
			{
				case 0:
					Bitmap_IDX = 4;
					break;
				case WALL:
					Bitmap_IDX = 3;
					break;
				case BOTTOM:
					Bitmap_IDX = 0;
					break;
				case GOAL:
					Bitmap_IDX = 5;
					break;
				default:
				{
					if (_MapHdr->MAP[Index] & PLAYER)
					{
						Bitmap_IDX = 6;
						_MapHdr->PlayerX = i;
						_MapHdr->PlayerY = j;

						break;
					}

					if (_MapHdr->MAP[Index] & BOX)
					{
						if (_MapHdr->MAP[Index] & GOAL)
						{
							Bitmap_IDX = 2;
							break;
						}

						Bitmap_IDX = 1;
						break;
					}

					break;
				}
			}

			SelectObject(hMemDC2, hBitmapArray[Bitmap_IDX]);
			BitBlt(hMemDC, i * BITMAP_SIZE, j * BITMAP_SIZE, BITMAP_SIZE, BITMAP_SIZE, hMemDC2, 0, 0, SRCCOPY);

			if (EditMode)
			{
				MoveToEx(hMemDC, i * BITMAP_SIZE, j * BITMAP_SIZE, NULL);
				LineTo(hMemDC, BMP_MAP_WIDTH, j * BITMAP_SIZE);

				MoveToEx(hMemDC, i * BITMAP_SIZE, 0, NULL);
				LineTo(hMemDC, i * BITMAP_SIZE, BMP_MAP_HEIGHT);
			}
		}
	}

	BitBlt(hDC, DrawX, 0, BMP_MAP_WIDTH, BMP_MAP_HEIGHT, hMemDC, 0, 0, SRCCOPY);

	DeleteObject(hMapPapaer);
	DeleteDC(hMemDC2);
	DeleteDC(hMemDC);
}

VOID ReleaseMap(MapHdr *_MapHdr)
{
	if (_MapHdr->MAP)
	{
		free(_MapHdr->MAP);
		_MapHdr->MAP = NULL;
	}
}


BOOL IsGoal(MapHdr *_MapHdr)
{
	UINT i;

	for (i = 0; i < _MapHdr->MAP_WIDTH * _MapHdr->MAP_HEIGHT; ++i)
	{
		if (_MapHdr->MAP[i] & GOAL)
		{
			if (_MapHdr->MAP[i] == (GOAL | BOX))
				continue;

			return 0;
		}
	}

	return 1;
}

BOOL IsHuman(MapHdr *_MapHdr)
{
	UINT i;

	for (i = 0; i < _MapHdr->MAP_WIDTH * _MapHdr->MAP_HEIGHT; ++i)
	{
		if (_MapHdr->MAP[i] & PLAYER)
			return 1;
	}

	return 0;
}

VOID CreateTool(HDC hDC, HBITMAP *hBitmapArray, UINT Select, UINT MaxRes)
{
	HBITMAP hToolPaper;
	HDC hPaperMemDC, hMemDC;
	HBRUSH hBr, hBr_Old;
	HPEN hPen, hPen_Old;
	UINT i, j;

	hToolPaper = CreateCompatibleBitmap(hDC, TOOL_WIDTH, TOOL_HEIGHT);
	hPaperMemDC = CreateCompatibleDC(hDC);
	hMemDC = CreateCompatibleDC(hPaperMemDC);

	SelectObject(hPaperMemDC, hToolPaper);

	hBr = CreateSolidBrush(0x0EC9FF);
	SelectObject(hPaperMemDC, hBr);
	Rectangle(hPaperMemDC, 0, 0, TOOL_WIDTH, TOOL_HEIGHT);
	hBr_Old = (HBRUSH)SelectObject(hPaperMemDC, hBr);

	for (i = 0, j = 16; i < MaxRes; ++i, j += 16)
	{
		SelectObject(hMemDC, hBitmapArray[i]);

		if (Select == i)
		{
			hPen = CreatePen(PS_SOLID, 2, 0xFF);
			hPen_Old = (HPEN)SelectObject(hPaperMemDC, hPen);
			Rectangle(hPaperMemDC, TOOL_WIDTH / 2 - 18, (i * BITMAP_SIZE) + j - 2, TOOL_WIDTH / 2 + 18, (i * BITMAP_SIZE) + j + 2 + BITMAP_SIZE);
			SelectObject(hPaperMemDC, hPen_Old);
			DeleteObject(hPen);
		}

		BitBlt(hPaperMemDC, TOOL_WIDTH / 2 - 16, (i * BITMAP_SIZE) + j, BITMAP_SIZE, BITMAP_SIZE, hMemDC, 0, 0, SRCCOPY);
	}

	BitBlt(hDC, 0, 0, TOOL_WIDTH, TOOL_HEIGHT, hPaperMemDC, 0, 0, SRCCOPY);

	DeleteDC(hMemDC);
	DeleteDC(hPaperMemDC);
	DeleteObject(hBr);
	DeleteObject(hToolPaper);
}

BOOL SelectTool(UINT X, UINT Y, UINT MaxRes, LPINT Select)
{
	UINT i, j;

	if (X >= 70 / 2 - 16 && X < 70 / 2 + 16)
	{
		for (i = 0, j = 16; i < MaxRes; ++i, j += 16)
		{
			if (Y >= (i * BITMAP_SIZE) + j && Y < (i * BITMAP_SIZE) + j + BITMAP_SIZE)
			{
				*Select = i;
				return 1;
			}
		}
	}

	return 0;
}

VOID GetRealWindowSizeSubClientWindowSize(HWND hWnd, SIZE *size)
{
	RECT rrt, crt;

	GetWindowRect(hWnd, &rrt);
	GetClientRect(hWnd, &crt);

	size->cx = (rrt.right - rrt.left) - crt.right;
	size->cy = (rrt.bottom - rrt.top) - crt.bottom;
}

VOID UpdateWindowSize(HWND hWnd, MapHdr *_MapHdr, BOOL EditMode)
{
	SIZE size;
	DWORD IncreaseY;

	GetRealWindowSizeSubClientWindowSize(hWnd, &size);

	IncreaseY = _MapHdr->MAP_HEIGHT * BITMAP_SIZE;

	if (!EditMode)
		IncreaseY += size.cy;
	else
	{
		if (IncreaseY < TOOL_HEIGHT)
			IncreaseY = TOOL_HEIGHT + size.cy;
		else
			IncreaseY = (IncreaseY - TOOL_HEIGHT) + TOOL_HEIGHT + size.cy;
	}

	SetWindowPos(hWnd, 
		NULL, 
		0, 
		0, 
		_MapHdr->MAP_WIDTH * BITMAP_SIZE + size.cx + (EditMode ? TOOL_WIDTH : 0),
		IncreaseY,
		SWP_NOMOVE | SWP_SHOWWINDOW
	);
}
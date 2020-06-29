#include <Windows.h>

BYTE Recorder[1000] = { 0 };
UINT Record_Conut = 0;
UINT Record_Index = 0;

VOID WriteMoving(char ax, char ay, BOOL touchBox)
{
	Record_Conut = Record_Index;

	if( ax == 0 && ay == -1 )
		Recorder[Record_Index] = touchBox * 4 + 1;
	else if( ax == 1 && ay == 0 )
		Recorder[Record_Index] = touchBox * 4 + 2;
	else if( ax == 0 && ay == 1 )
		Recorder[Record_Index] = touchBox * 4 + 3;
	else if( ax == -1 && ay == 0 )
		Recorder[Record_Index] = touchBox * 4 + 4;
	
	++Record_Index;
	++Record_Conut;
}

VOID ReadMoving(char *ax, char *ay, BOOL *touchBox)
{
	BYTE Moving;

	*ax = 0;
	*ay = 0;
	
	if (*touchBox = (Recorder[Record_Index] > 4))
		Moving = Recorder[Record_Index] - 4;
	else
		Moving = Recorder[Record_Index];

	switch(Moving)
	{
		case 1:
			*ay = -1;
			break;
		case 2:
			*ax = 1;
			break;
		case 3:
			*ay = 1;
			break;
		case 4:
			*ax = -1;
			break;
	}
}

VOID DeleteMoving()
{
	memset(Recorder, 0, sizeof(Recorder));
	Record_Conut = 0;
	Record_Index = 0;
}
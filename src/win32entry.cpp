/*  +======| File Info |===============================================================+
    |                                                                                  |
    |     Subdirectory:  /src                                                          |
    |    Creation date:  2/28/2021 1:58:15 AM                                          |
    |    Last Modified:                                                                |
    |                                                                                  |
    +=====================| Sayed Abid Hashimi, Copyright � All rights reserved |======+  */

#include "windows.h"

typedef int int32;
typedef unsigned int uint32;
typedef int bool32;
typedef float real32;

#define internal static

#define MAX_STRING 1024

#include "commons.h"
#include "lexer.cpp"

// NOTE(Khisrow): Globals
HANDLE GLOBALConsoleOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
HANDLE GLOBALConsoleInputHandle = GetStdHandle(STD_INPUT_HANDLE);

internal void
Win32StdOut(char *Text)
{
	DWORD CharRead;
	WriteConsole(GLOBALConsoleOutputHandle, Text, StringLength(Text), NULL, NULL);
}

// TODO(Khisrow): The Input takes only 1024 characters, should it be increased?
struct win32_console_stdin
{
	char *Input;
	DWORD CharRead;
};

internal DWORD
Win32StdIn(char *String = 0, uint32 StringSize = 0)
{
	char Temp[4];
	if(!String)
	{
		String = Temp;
		StringSize = 4;
	}
	else Assert(StringSize != 0);

	String[0] = '\0';
	DWORD CharRead = 0;
	ReadConsole(GLOBALConsoleInputHandle, String,
				StringSize, &CharRead, NULL);
	if(IndexInString(String, '\r'))
	{
		String[IndexInString(String, '\r')] = '\0';
	}
	else
	{
		String[IndexInString(String, '\n') + 1] = '\0';
	}

	return CharRead;
}

#if 0
BOOL WINAPI
HandlerRoutine(DWORD CtrlType)
{
	switch (CtrlType)
	{
		case CTRL_C_EVENT:
		{
			Beep(750, 300);
			return true;
		}

		default:
		{
			return false;
		}
	}
}
#endif

internal void
ClearScreen()
{
	CONSOLE_SCREEN_BUFFER_INFO ConBufferInfo = {};
	GetConsoleScreenBufferInfo(GLOBALConsoleOutputHandle, &ConBufferInfo);

	SMALL_RECT RectToMove = {};
	RectToMove.Left = 0;
	RectToMove.Top = 0;
	RectToMove.Right = ConBufferInfo.dwSize.X;
	RectToMove.Bottom = ConBufferInfo.dwSize.X;

	COORD BufferOrigin = {};
	BufferOrigin.X = -ConBufferInfo.dwSize.X;
	BufferOrigin.Y = -ConBufferInfo.dwSize.Y;

	CHAR_INFO CharFill = {};
	CharFill.Attributes = 0;
	CharFill.Char.AsciiChar = (char)' ';

	ScrollConsoleScreenBuffer(GLOBALConsoleOutputHandle, &RectToMove, NULL, BufferOrigin, &CharFill);
	COORD Cursor = {};
	SetConsoleCursorPosition(GLOBALConsoleOutputHandle, Cursor);
}

int main(int argc, char *argv[])
{
	// NOTE(Khisrow): Input Strings
	char String[MAX_STRING];
	char String2[MAX_STRING];

	if(GLOBALConsoleOutputHandle == INVALID_HANDLE_VALUE) return 0;
	if(GLOBALConsoleInputHandle == INVALID_HANDLE_VALUE) return 0;

	//NOTE(Khisrow): Token Memory
	lexer_state LexerState = {};
	LexerState.Tokens.MemorySize = Megabytes(10);
	LexerState.Tokens.MemoryBase = (token *)VirtualAlloc(0, LexerState.Tokens.MemorySize,
														 MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

	//NOTE(Khisrow): Text Memory
	uint32 TextSize = Megabytes(10);
	char *TextMemory = (char *)VirtualAlloc(0, TextSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

	//if(StringCompare(argv[1], "shell"))
	{
		ClearScreen();
		SetConsoleTitle("Project Darya Shell");
		SetConsoleCtrlHandler(NULL, true);

		win32_console_stdin ReadData = {};
		ReadData.Input = TextMemory;
		// Win32StdOut("Project Darya Shell Module\n\n");
		while(true)
		{
			Win32StdOut("Project Darya Shell >> ");
			ReadData.CharRead = Win32StdIn(TextMemory, TextSize);

			if(StringCompare(ReadData.Input, "exit"))
			{
				Win32StdOut("\nPress Enter to exit...");
				Win32StdIn();
				break;
			}
			else if(StringCompare(ReadData.Input, "clear"))
			{
				ClearScreen();
			}
			else
			{
				if(ReadData.CharRead == 0) Win32StdOut("\nKeyboard Interrupt voided!\n\n");
				else if(StringCompare(ReadData.Input, "\r\n")) {}
				else
				{
					char FileName[MAX_PATH] = "<stdin>"; while(FileName[0] == 1) return 0;
					InitializeLexer(&LexerState, FileName, TextMemory);
					op_status LexerStatus = PopulateTokens(&LexerState);
					if(LexerStatus.Success)
					{
						for(token *Token = LexerState.Tokens.MemoryBase;
							Token->Type != TT_EOF;
							++Token)
						{
							Concat(String, true, " [", Token->Value, " : ", TokenTypeString[Token->Type], "] ");
						}
						Concat(String, true, "\n");
						Win32StdOut(String);
					}
					else 
					{
						Concat(String, false, LexerStatus.Error.Message, "\n");
						Win32StdOut(String);
					}
				}
			}
		}
	}
}

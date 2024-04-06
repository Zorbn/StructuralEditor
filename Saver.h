#pragma once

#include "Writer.h"

typedef struct Block Block;

typedef struct Saver
{
    Writer writer;
} Saver;

Saver SaverNew(void);
void SaverDelete(Saver *saver);
void SaverReset(Saver *saver);
void SaverSave(Saver *saver, Block *block);

void SaverSavePin(Saver *saver, Block *block);
void SaverSaveDo(Saver *saver, Block *block);
void SaverSaveStatementList(Saver *saver, Block *block);
void SaverSaveFunctionHeader(Saver *saver, Block *block);
void SaverSaveFunction(Saver *saver, Block *block);
void SaverSaveLambdaFunctionHeader(Saver *saver, Block *block);
void SaverSaveLambdaFunction(Saver *saver, Block *block);
void SaverSaveCase(Saver *saver, Block *block);
void SaverSaveIfCases(Saver *saver, Block *block);
void SaverSaveElseCase(Saver *saver, Block *block);
void SaverSaveIf(Saver *saver, Block *block);
void SaverSaveAssignment(Saver *saver, Block *block);
void SaverSaveAdd(Saver *saver, Block *block);
void SaverSaveCall(Saver *saver, Block *block);
void SaverSaveIdentifier(Saver *saver, Block *block);
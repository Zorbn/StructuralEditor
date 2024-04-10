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
void SaverSaveAssign(Saver *saver, Block *block);
void SaverSaveComment(Saver *saver, Block *block);
void SaverSaveExpressionList(Saver *saver, Block *block);
void SaverSaveNot(Saver *saver, Block *block);
void SaverSaveLength(Saver *saver, Block *block);
void SaverSaveConcatenate(Saver *saver, Block *block);
void SaverSaveModulo(Saver *saver, Block *block);
void SaverSaveDivide(Saver *saver, Block *block);
void SaverSaveMultiply(Saver *saver, Block *block);
void SaverSaveAdd(Saver *saver, Block *block);
void SaverSaveSubtract(Saver *saver, Block *block);
void SaverSaveGreaterEqual(Saver *saver, Block *block);
void SaverSaveLessEqual(Saver *saver, Block *block);
void SaverSaveGreater(Saver *saver, Block *block);
void SaverSaveLess(Saver *saver, Block *block);
void SaverSaveNotEqual(Saver *saver, Block *block);
void SaverSaveEqual(Saver *saver, Block *block);
void SaverSaveAnd(Saver *saver, Block *block);
void SaverSaveOr(Saver *saver, Block *block);
void SaverSaveCall(Saver *saver, Block *block);
void SaverSaveIdentifier(Saver *saver, Block *block);
void SaverSaveForLoop(Saver *saver, Block *block);
void SaverSaveForLoopCondition(Saver *saver, Block *block);
void SaverSaveForLoopBounds(Saver *saver, Block *block);
void SaverSaveForInLoopCondition(Saver *saver, Block *block);
void SaverSaveWhileLoop(Saver *saver, Block *block);
void SaverSaveReturn(Saver *saver, Block *block);
void SaverSaveLocal(Saver *saver, Block *block);
void SaverSaveTable(Saver *saver, Block *block);
void SaverSaveTableKeyValuePair(Saver *saver, Block *block);
void SaverSaveTableExpressionValuePair(Saver *saver, Block *block);
void SaverSaveTableValue(Saver *saver, Block *block);
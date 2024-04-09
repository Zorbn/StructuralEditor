#include "Saver.h"
#include "Block.h"

Saver SaverNew(void)
{
    return (Saver){
        .writer = WriterNew(),
    };
}

void SaverDelete(Saver *saver)
{
    WriterDelete(&saver->writer);
}

void SaverReset(Saver *saver)
{
    WriterReset(&saver->writer);
}

void SaverSave(Saver *saver, Block *block)
{
    BlockKind *kind = &BlockKinds[block->kindId];

    kind->save(saver, block);
}

static void SaverSaveBlockList(Saver *saver, Block *block, int32_t firstI, char *seperator)
{
    int32_t childrenCount = BlockGetChildrenCount(block);

    for (int32_t i = firstI; i < childrenCount; i++)
    {
        Block *child = BlockGetChild(block, i);

        SaverSave(saver, child);

        if (i < childrenCount - 1)
        {
            WriterWrite(&saver->writer, seperator);
        }
    }
}

void SaverSavePin(Saver *saver, Block *block)
{
    (void)saver, (void)block;
}

void SaverSaveDo(Saver *saver, Block *block)
{
    WriterWriteLine(&saver->writer, "do");
    WriterIndent(&saver->writer);

    int32_t childrenCount = BlockGetChildrenCount(block);

    for (int32_t i = 0; i < childrenCount; i++)
    {
        Block *child = BlockGetChild(block, i);

        SaverSave(saver, child);
        WriterNewline(&saver->writer);
    }

    WriterUnindent(&saver->writer);
    WriterWrite(&saver->writer, "end");
}

void SaverSaveStatementList(Saver *saver, Block *block)
{
    int32_t childrenCount = BlockGetChildrenCount(block);

    for (int32_t i = 0; i < childrenCount; i++)
    {
        Block *child = BlockGetChild(block, i);

        SaverSave(saver, child);
        WriterNewline(&saver->writer);
    }
}

void SaverSaveFunctionHeader(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "function ");
    SaverSave(saver, BlockGetChild(block, 0));
    WriterWrite(&saver->writer, "(");
    SaverSaveBlockList(saver, block, 1, ", ");
    WriterWriteLine(&saver->writer, ")");
}

void SaverSaveFunction(Saver *saver, Block *block)
{
    SaverSave(saver, BlockGetChild(block, 0));
    WriterIndent(&saver->writer);
    SaverSave(saver, BlockGetChild(block, 1));
    WriterUnindent(&saver->writer);
    WriterWrite(&saver->writer, "end");
}

void SaverSaveLambdaFunctionHeader(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "function ");
    WriterWrite(&saver->writer, "(");
    SaverSaveBlockList(saver, block, 0, ",");
    WriterWriteLine(&saver->writer, ")");
}

void SaverSaveLambdaFunction(Saver *saver, Block *block)
{
    SaverSave(saver, BlockGetChild(block, 0));
    WriterIndent(&saver->writer);
    SaverSave(saver, BlockGetChild(block, 1));
    WriterUnindent(&saver->writer);
    WriterWriteLine(&saver->writer, "end");
}

void SaverSaveCase(Saver *saver, Block *block)
{
    SaverSave(saver, BlockGetChild(block, 0));
    WriterWriteLine(&saver->writer, " then");

    WriterIndent(&saver->writer);

    int32_t childrenCount = BlockGetChildrenCount(block);

    for (int32_t i = 1; i < childrenCount; i++)
    {
        Block *child = BlockGetChild(block, i);

        SaverSave(saver, child);
        WriterNewline(&saver->writer);
    }

    WriterUnindent(&saver->writer);
}

void SaverSaveIfCases(Saver *saver, Block *block)
{
    int32_t childrenCount = BlockGetChildrenCount(block);

    for (int32_t i = 0; i < childrenCount; i++)
    {
        if (i == 0)
        {
            WriterWrite(&saver->writer, "if ");
        }
        else
        {
            WriterWrite(&saver->writer, "elseif ");
        }

        Block *child = BlockGetChild(block, i);

        SaverSave(saver, child);
    }
}

void SaverSaveElseCase(Saver *saver, Block *block)
{
    WriterWriteLine(&saver->writer, "else");
    WriterIndent(&saver->writer);

    int32_t childrenCount = BlockGetChildrenCount(block);

    for (int32_t i = 0; i < childrenCount; i++)
    {
        Block *child = BlockGetChild(block, i);

        SaverSave(saver, child);
        WriterNewline(&saver->writer);
    }

    WriterUnindent(&saver->writer);
}

void SaverSaveIf(Saver *saver, Block *block)
{
    SaverSave(saver, BlockGetChild(block, 0));

    Block *elseBlock = BlockGetChild(block, 1);

    if (BlockContainsNonPin(elseBlock))
    {
        SaverSave(saver, elseBlock);
    }

    WriterWrite(&saver->writer, "end");
}

void SaverSaveAssignment(Saver *saver, Block *block)
{
    SaverSave(saver, BlockGetChild(block, 0));
    WriterWrite(&saver->writer, " = ");
    SaverSave(saver, BlockGetChild(block, 1));
}

void SaverSaveNot(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "not ");
    SaverSave(saver, BlockGetChild(block, 0));
}

void SaverSaveLength(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "#");
    SaverSave(saver, BlockGetChild(block, 0));
}

void SaverSaveConcatenate(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "(");
    SaverSaveBlockList(saver, block, 0, " .. ");
    WriterWrite(&saver->writer, ")");
}

void SaverSaveModulo(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "(");
    SaverSaveBlockList(saver, block, 0, " % ");
    WriterWrite(&saver->writer, ")");
}

void SaverSaveDivide(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "(");
    SaverSaveBlockList(saver, block, 0, " / ");
    WriterWrite(&saver->writer, ")");
}

void SaverSaveMultiply(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "(");
    SaverSaveBlockList(saver, block, 0, " * ");
    WriterWrite(&saver->writer, ")");
}

void SaverSaveAdd(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "(");
    SaverSaveBlockList(saver, block, 0, " + ");
    WriterWrite(&saver->writer, ")");
}

void SaverSaveSubtract(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "(");
    SaverSaveBlockList(saver, block, 0, " - ");
    WriterWrite(&saver->writer, ")");
}

void SaverSaveGreaterEqual(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "(");
    SaverSave(saver, BlockGetChild(block, 0));
    WriterWrite(&saver->writer, " >= ");
    SaverSave(saver, BlockGetChild(block, 1));
    WriterWrite(&saver->writer, ")");
}

void SaverSaveLessEqual(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "(");
    SaverSave(saver, BlockGetChild(block, 0));
    WriterWrite(&saver->writer, " <= ");
    SaverSave(saver, BlockGetChild(block, 1));
    WriterWrite(&saver->writer, ")");
}

void SaverSaveGreater(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "(");
    SaverSave(saver, BlockGetChild(block, 0));
    WriterWrite(&saver->writer, " > ");
    SaverSave(saver, BlockGetChild(block, 1));
    WriterWrite(&saver->writer, ")");
}

void SaverSaveLess(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "(");
    SaverSave(saver, BlockGetChild(block, 0));
    WriterWrite(&saver->writer, " < ");
    SaverSave(saver, BlockGetChild(block, 1));
    WriterWrite(&saver->writer, ")");
}

void SaverSaveNotEqual(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "(");
    SaverSaveBlockList(saver, block, 0, " != ");
    WriterWrite(&saver->writer, ")");
}

void SaverSaveEqual(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "(");
    SaverSaveBlockList(saver, block, 0, " == ");
    WriterWrite(&saver->writer, ")");
}

void SaverSaveAnd(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "(");
    SaverSaveBlockList(saver, block, 0, " and ");
    WriterWrite(&saver->writer, ")");
}

void SaverSaveOr(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "(");
    SaverSaveBlockList(saver, block, 0, " or ");
    WriterWrite(&saver->writer, ")");
}

void SaverSaveCall(Saver *saver, Block *block)
{
    SaverSave(saver, BlockGetChild(block, 0));

    WriterWrite(&saver->writer, "(");

    SaverSaveBlockList(saver, block, 1, ", ");

    WriterWrite(&saver->writer, ")");
}

void SaverSaveIdentifier(Saver *saver, Block *block)
{
    WriterWriteIdentifier(&saver->writer, block->data.identifier.text);
}

void SaverSaveForLoop(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "for ");
    SaverSave(saver, BlockGetChild(block, 0));
    WriterWriteLine(&saver->writer, " do");

    WriterIndent(&saver->writer);
    SaverSave(saver, BlockGetChild(block, 1));
    WriterUnindent(&saver->writer);

    WriterWrite(&saver->writer, "end");
}

void SaverSaveForLoopCondition(Saver *saver, Block *block)
{
    SaverSave(saver, BlockGetChild(block, 0));
    WriterWrite(&saver->writer, " = ");
    SaverSave(saver, BlockGetChild(block, 1));
}

void SaverSaveForLoopBounds(Saver *saver, Block *block)
{
    SaverSave(saver, BlockGetChild(block, 0));
    WriterWrite(&saver->writer, ", ");
    SaverSave(saver, BlockGetChild(block, 1));

    Block *stepBlock = BlockGetChild(block, 2);

    if (stepBlock->kindId != BlockKindIdPin)
    {
        WriterWrite(&saver->writer, ", ");
        SaverSave(saver, BlockGetChild(block, 2));
    }
}

void SaverSaveForInLoopCondition(Saver *saver, Block *block)
{
    SaverSave(saver, BlockGetChild(block, 0));
    WriterWrite(&saver->writer, " in ");
    SaverSave(saver, BlockGetChild(block, 1));
}

void SaverSaveWhileLoop(Saver *saver, Block *block)
{
    WriterWrite(&saver->writer, "while ");
    SaverSave(saver, BlockGetChild(block, 0));
    WriterWriteLine(&saver->writer, " do");

    WriterIndent(&saver->writer);
    SaverSave(saver, BlockGetChild(block, 1));
    WriterUnindent(&saver->writer);

    WriterWrite(&saver->writer, "end");
}

void SaverSaveTable(Saver *saver, Block *block)
{
    WriterWriteLine(&saver->writer, "{");
    WriterIndent(&saver->writer);

    int32_t childrenCount = BlockGetChildrenCount(block);

    for (int32_t i = 0; i < childrenCount; i++)
    {

        Block *child = BlockGetChild(block, i);

        SaverSave(saver, child);
    }

    WriterUnindent(&saver->writer);
    WriterWrite(&saver->writer, "}");
}

void SaverSaveTableKeyValuePair(Saver *saver, Block *block)
{
    if (!BlockContainsNonPin(block))
    {
        return;
    }

    SaverSave(saver, BlockGetChild(block, 0));
    WriterWrite(&saver->writer, " = ");
    SaverSave(saver, BlockGetChild(block, 1));
    WriterWriteLine(&saver->writer, ",");
}

void SaverSaveTableExpressionValuePair(Saver *saver, Block *block)
{
    if (!BlockContainsNonPin(block))
    {
        return;
    }

    WriterWrite(&saver->writer, "[");
    SaverSave(saver, BlockGetChild(block, 0));
    WriterWrite(&saver->writer, "] = ");
    SaverSave(saver, BlockGetChild(block, 1));
    WriterWriteLine(&saver->writer, ",");
}

void SaverSaveTableValue(Saver *saver, Block *block)
{
    if (!BlockContainsNonPin(block))
    {
        return;
    }

    SaverSave(saver, BlockGetChild(block, 0));
    WriterWriteLine(&saver->writer, ",");
}
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
    }

    WriterUnindent(&saver->writer);
    WriterWriteLine(&saver->writer, "end");
}

void SaverSaveStatementList(Saver *saver, Block *block)
{
    int32_t childrenCount = BlockGetChildrenCount(block);

    for (int32_t i = 0; i < childrenCount; i++)
    {
        Block *child = BlockGetChild(block, i);

        SaverSave(saver, child);
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
    WriterWriteLine(&saver->writer, "end");
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

    WriterWriteLine(&saver->writer, "end");
}

void SaverSaveAssignment(Saver *saver, Block *block)
{
    SaverSave(saver, BlockGetChild(block, 0));
    WriterWrite(&saver->writer, " = ");
    SaverSave(saver, BlockGetChild(block, 1));
    WriterNewline(&saver->writer);
}

void SaverSaveAdd(Saver *saver, Block *block)
{
    SaverSaveBlockList(saver, block, 0, " + ");
}

void SaverSaveCall(Saver *saver, Block *block)
{
    SaverSave(saver, BlockGetChild(block, 0));

    WriterWrite(&saver->writer, "(");

    SaverSaveBlockList(saver, block, 1, ", ");

    WriterWriteLine(&saver->writer, ")");
}

void SaverSaveIdentifier(Saver *saver, Block *block)
{
    WriterWriteIdentifier(&saver->writer, block->data.identifier.text);
}
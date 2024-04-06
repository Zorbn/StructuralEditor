#include "Writer.h"

Writer WriterNew(void)
{
    return (Writer){
        .text = ListNew_char(1024),
    };
}

void WriterDelete(Writer *writer)
{
    ListDelete_char(&writer->text);
}

void WriterReset(Writer *writer)
{
    ListReset_char(&writer->text);
}

void WriterNewline(Writer *writer)
{
    ListPush_char(&writer->text, '\n');
    writer->isAfterNewline = true;
}

static void WriterWriteInternal(Writer *writer, char *string, bool isIdentifier)
{
    if (writer->isAfterNewline)
    {
        for (int32_t i = 0; i < writer->indentCount; i++)
        {
            ListPush_char(&writer->text, '\t');
        }

        writer->isAfterNewline = false;
    }

    if (!string)
    {
        return;
    }

    bool doConvert = isIdentifier && string[0] != '"' && string[0] != '\'';

    for (int32_t i = 0; string[i] != '\0'; i++)
    {
        if (string[i] == ' ' && doConvert)
        {
            ListPush_char(&writer->text, '_');
            continue;
        }

        ListPush_char(&writer->text, string[i]);
    }
}

void WriterWrite(Writer *writer, char *string)
{
    WriterWriteInternal(writer, string, false);
}

void WriterWriteIdentifier(Writer *writer, char *string)
{
    WriterWriteInternal(writer, string, true);
}

void WriterWriteLine(Writer *writer, char *string)
{
    WriterWrite(writer, string);
    WriterNewline(writer);
}

void WriterIndent(Writer *writer)
{
    writer->indentCount += 1;
}

void WriterUnindent(Writer *writer)
{
    writer->indentCount -= 1;
}
#pragma once

#include "List.h"

#include <stdbool.h>

typedef struct Writer
{
    List_char text;
    bool isAfterNewline;
    int32_t indentCount;
} Writer;

Writer WriterNew(void);
void WriterDelete(Writer *writer);
void WriterReset(Writer *writer);
void WriterNewline(Writer *writer);
void WriterWrite(Writer *writer, char *string);
void WriterWriteIdentifier(Writer *writer, char *string);
void WriterWriteLine(Writer *writer, char *string);
void WriterIndent(Writer *writer);
void WriterUnindent(Writer *writer);
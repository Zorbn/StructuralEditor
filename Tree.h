#pragma once

typedef struct Tree
{
    int updatedY;
} Tree;

Tree TreeNew();
void TreeRequestUpdate(Tree *tree, int y);
#include "Tree.h"
#include "Math.h"

Tree TreeNew()
{
    return (Tree){
        .updatedY = 0,
    };
}

#include <stdio.h>

void TreeRequestUpdate(Tree *tree, int y)
{
    tree->updatedY = MathInt32Min(tree->updatedY, y);
    printf("%d\n", tree->updatedY);
}
#pragma once

#include "Input.h"
#include "Camera.h"
#include "Theme.h"
#include "Font.h"

typedef enum SearchBarState
{
    SearchBarStateIdle,
    SearchBarStateUpdated,
    SearchBarStateCancel,
    SearchBarStateConfirm,
} SearchBarState;

typedef char *CharPointer;
ListDefine(CharPointer);

typedef struct SearchBar
{
    List_char text;
    List_CharPointer results;
} SearchBar;

SearchBar SearchBarNew(void);
void SearchBarDelete(SearchBar *searchBar);
SearchBarState SearchBarUpdate(SearchBar *searchBar, Input *input);
void SearchBarReset(SearchBar *searchBar);
bool SearchBarTryAddResult(SearchBar *searchBar, char *result);
void SearchBarClearSearchResults(SearchBar *searchBar);
void SearchBarDraw(SearchBar *searchBar, Camera *camera, Font *font, Theme *theme);
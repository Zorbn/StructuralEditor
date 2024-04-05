#pragma once

#include "Input.h"
#include "Camera.h"
#include "Theme.h"
#include "Font.h"

typedef enum SearchBarState
{
    SearchBarStateInProgress,
    SearchBarStateCancel,
    SearchBarStateConfirm,
} SearchBarState;

typedef struct SearchBar
{
    List_char text;
} SearchBar;

SearchBar SearchBarNew(void);
void SearchBarDelete(SearchBar *searchBar);
SearchBarState SearchBarUpdate(SearchBar *searchBar, Input *input);
void SearchBarReset(SearchBar *searchBar);
void SearchBarDraw(SearchBar *searchBar, Camera *camera, Font *font, Theme *theme);
#ifndef MENU_HPP
#define MENU_HPP

#include <String>

/* externs */
extern int mainCursor;
extern int subCursor;
extern int optCursor;

/* Function protos */
String tr(const char* en, const char* cz);
String getSubmenuItemText(int cat, int idx);
void drawMenu();
void runMenu();
void runApp();

#endif
# extCurses - nCurses extended

extCurses is a nCurses widget library to make creating GUI's easier. At the moment only a horizontal menu and subitems are implemented.


## API Usage

### Menu Widget

To **initialize** a menu widget include the file menu.h. The function **menu_init(int y_start, int x_start, char\*\* menu_subitems_ptr, int flags)** must be called first with the following arguments:

-*y\_start, x_start*: y and x positions of the menu on the screen.

-*menu\_subitems_ptr*: Array of strings containing information about each menu and subitem items (if present). The first string element will be the first menu's item's text. The following string elements will be this item's subitems' texts. The last element must be 0. Repeat for more menu and subitem items. Example: {"menuItem1", "subitem 1", "subitem 2", 0, "menuItem2", "subitem 1", 0}. See code example below.

-*flags*: Styling flags. See menu.h for definitions. Flags can be OR'd together.

The function menu_init returns a MENU\_PTR,  a pointer to a MENU structure which will be used as argument for other menu configuration functions. If MENU\_PTR is NULL, an **error** occured. An internal variable, menu\_err, holds the last error's code. menu.h contains the error constants. To print the error description to standard error use **menu\_perror(char\* message)**. *message* will be printed before the error description if present. 

To **draw** the menu on the screen call **menu_show(MENU_PTR menuPtr);**, where menuPtr is the pointer to the MENU structure returned from a menu_init call.

To create underline **shortcuts** call **menu_shortcut(MENU_PTR menuPtr , int menu, int subitem, char shortcutChar)** for each shortcut desired with the following arguments:

-*menuPtr*: MENU* returned from a menu_init() call.

-*menu*: index that selects the menu item. Starts at 0.

-*subitem*: index that selects subitem option. Starts at 0. If subitem is -1, the shortcut is added to menu option selected with the menu argument.

-*shortcutChar*: menu's or subitem's character that is to be underlined.

menu_shortcut returns 0 if either: shortcutChar is not a character in the alphabet, the menu and subitem arguments are not valid (menu or subitem index does not exist) or the shortcutChar has already been assigned to another menu or subitem item.

To **style** a menu call **menu_style(MENU_PTR menuPtr, int flags)**. The argument *menuPtr* is the same used in menu\_shortcut. *flags* is the same used in menu\_init.


To **navigate** the menu the user must use the arrow keys. To **select** a menu or subitem item, the user must press either the ESC key, the ENTER key or a menu shortcut. To pass control to the menu, thus allowing navigation and selection,  **menu\_action(MENU_PTR menuPtr, int\* menuChoice, int\* subChoice)** must be called with the following arguments:

-*menuPtr*: Pointer to the menu, returned from a menu_init call, to which the action is to be performed.

-*menuChoice*: index of the menu item the user selected. Starts at 0.

-*subChoice*: index of the subitem item the user selected. Starts at 0.

menu_action will block until any of the following actions occur: 
-the ESC key is pressed, in which case 0 is returned indicating no selection was performed. 
-the ENTER key or a shortcut key is pressed, in which case

To **unallocate** the resources used for a menu call **menu_end(MENU_PTR menuPtr)**.


## Code Example

### Screenshot
![screenshot](https://cloud.githubusercontent.com/assets/20986580/21282915/80f52428-c3c8-11e6-88b9-30dc1cc95a17.png)
### Code

```C
#include <ncurses.h>
#include <stdlib.h>
#include "./extcurses/menu.h"


int main(int argc, char** argv)
{
	WINDOW* mainwnd;
	MENU* topmenu;
	int choice;
	char ch;

	int menuChoice, subChoice;

	//Create menu on a separate function, so that this array won't occupy main's stack?
	char* topMenuOpts[] = {
		"Database", "Create", "Change","Delete",0,		//1st menu with 3 subitems
		"Table", "Add", "Delete", 0,				//2nd menu with 2 subitems
		"View", "sub1", "sub2", 0,				//3rd menu with 2 subitems
	0 };

	//Setup ncurses main standard screen and input
	mainwnd = initscr();
	crmode();
	raw();
	noecho();
	keypad(mainwnd, TRUE);
	
	//Initialize menu
	topmenu = menu_init(0, 0, topMenuOpts, SPACE_WIDE | TO_END ); //0,0 initial positions

	if(topmenu == NULL)
	{
		menu_perror("ERROR");
		exit(EXIT_FAILURE);
	}

	menu_shortcut(topmenu, 0, -1, 'D');	//Make letter 'D' keyboard shortcut for "Database" menu item
	menu_shortcut(topmenu, 0, 0, 'C'); 	//'C' shortcut for Database -> Create subitem
	menu_shortcut(topmenu, 0, 1, 'h'); 	//'h' shortcut for Database -> Change
	menu_shortcut(topmenu, 1, -1, 'T');	//'T' shortcut for Table menu item
	menu_shortcut(topmenu, 2, -1, 'e');	//'e' shortcut for View menu item

	clear();		//Clear screen
	menu_show(topmenu);	//Draw menu on screen	

	ch = 'a';
	while(ch != 'q')
	{
		ch = getch();	
		if(ch == ESCAPE_KEY)
			menu_action(topmenu, &menuChoice, &subChoice);	//Pass control to menu and get user choice

		mvprintw(30,0,"Choice is : menu: %d\tSubitem: %d", menuChoice, subChoice);	//Print choice for debugging
		refresh();
		
		if(menuChoice == 0) //Database menu item
		{
			switch(subChoice)
			{
				case 0:	//Database->Create
				case 1:	//Database->Change
				case 2: //Database->Delete
					break;
			}
		}	

		else if(menuChoice == 1) //Table menu item
		{
			switch(subChoice)
			{
				case 0:	//Table->Add
				case 1:	//Table->Delete
					break;
			}
		}	

		else {} //View menu item
		
		//touchwin(stdscr);
		//refresh();
		menu_show(topmenu);	//Redraw menu
	}
	
	noraw();		
	echo();
	
	menu_end(topmenu);	//Deallocate resources for menu
	endwin();

	exit(0);
}

```

## Instalation and compilation

Run make to create the extcurses.a static library file.

Use the extcurses.a static library along with the ncurses dynamic library on compilation. Example: 
gcc -o appName appName.o -lncurses extcurses.a

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <termios.h>
#include "menu.h"


/***************************************************************************
 *Menu creation and manipulation functions
 ***************************************************************************
 */

int menu_err = 0;	//Contains errors. Definitions include file

/*-----------------------------------------------------------------------
 *Allocate space for a MENU structure and initialize it using arguments.
 *y_start, x_start: starting y and x positions
 *menu_sub_ptr: contains menus and its associated submenus. Each 
 *menu /submenu group ends with a 0
 *flags: definitions to draw window
 *------------------------------------------------------------------------
 */
MENU_PTR menu_init(int y_start, int x_start, char** menu_sub_ptr, int flags)
{
	MENU_PTR newmenu;
	WINDOW* temp;
	int numcols, numopt; //Number of columns to fit all menus. # of options
	char** opt_ptr;

	newmenu = (MENU_PTR)malloc(sizeof(MENU));
	if(newmenu == NULL)
	{
		menu_err |= ERROR_ALLOC_MENU;
		return newmenu;
	}
	memset(newmenu, 0, sizeof(MENU));

	newmenu->start_x = x_start;
	newmenu->start_y = y_start;

	//Calculate spacing between options
	if(!(flags & (SPACE_NARROW | SPACE_WIDE))) //Default option	
		newmenu->spacepads = 1;
	else
	{
		if(flags & SPACE_NARROW)
			newmenu->spacepads  = NARROW_SPACES;
		else
			newmenu->spacepads = WIDE_SPACES;
	}

	//Calculate number of  columns required to fill menus
	//and fill MENU.option structure
	opt_ptr = menu_sub_ptr;
	numcols = numopt = 0;	
	if(*opt_ptr !=  0) //If menu is not empty
	{
		int cursub = 0;
		//Register first option
		strncpy(newmenu->opt[numopt].menuname, menu_sub_ptr[numopt], MAX_LENGTH);
		numopt++;
		numcols += strlen(*opt_ptr);
		opt_ptr++;
			
		//Register the rest of them if present
		while(numopt < MAX_OPTIONS)
		{
			if(*opt_ptr == 0) //whole menu, or submenu end detected
			{
				newmenu->opt[numopt-1].numsubs = cursub;
				opt_ptr++;
				if(*opt_ptr == 0) //whole menu ended	
					break;
				else //only submenu ended. new option incoming
				{
					numopt++;
					strncpy(newmenu->opt[numopt-1].menuname,*opt_ptr,MAX_LENGTH);
					numcols += strlen(*opt_ptr) + newmenu->spacepads;
					cursub = 0;
				}
			}	

			else //Submenu
			{
				//CHeck if too many sub menus
				if(cursub == MAX_SUB)
				{
					menu_err |= ERROR_MANY_SUBS;	
					return NULL;
				}
				
				strncpy(newmenu->opt[numopt-1].subs[cursub], *opt_ptr, MAX_LENGTH);
				cursub++;
			}

			opt_ptr++;
		}
		
		//If OPT_BAR option is enabled, increment number of columns to fit bars
		//OPT_BAR adds 3 spaces between options
		if(flags & OPT_BAR)
			numcols += (3*(numopt-1));
	}
	
	//Check if menu is to large for screen
	if((numcols + x_start) > COLS)
	{
		menu_err |= ERROR_TOO_WIDE;
		return NULL;
	}

	if(flags & TO_END)
		numcols = COLS - x_start;

	newmenu->numopts = numopt;
	newmenu->flags = flags;	

	temp = newwin(1, numcols, y_start, x_start);
	if(temp == NULL)
	{
		menu_err |= ERROR_ALLOC_WINDOW;
		free(newmenu);
		return NULL;
	}
	newmenu->menuwnd = temp;
		
	return newmenu;
}

/*-----------------------------------
 *return 1 if ch is alpha, 0 otherwise
 *------------------------------------
 */
static int isal(char ch)
{
	int alpha=0;
	if((ch > 'a' && ch < 'z' ) || (ch > 'A' && ch <'Z'))
		alpha = 1;
	return alpha;
}

/*----------------------------------------------------------
 *Check if shortcut is already being used in menu.
 *return 1 if it is, 0  otherwise
 *----------------------------------------------------------
 */
static int isredundant(MENU_PTR menu, char shortcut, int menuindex, int subindex)
{
	int index;

	//Submenu
	if(subindex >= 0)
	{
		int numSubs = menu->opt[menuindex].numsubs;
		for(index = 0; index < numSubs; index++)
			if(menu->opt[menuindex].shortcuts[index] == shortcut)
				return 1;
	}
	
	//Menu
	else
	{
		int numOpts = menu->numopts;
		for(index = 0; index < numOpts; index++)
			if(menu->opt[index].shortcut == shortcut)
				return 1;
	}
	return 0;
}

/*---------------------------------------------------------
 *Return 1 if menu with index menuIndex and submenu with index
 *subIndex exist. 0 otherwise.
 *---------------------------------------------------------
 */
static int indexarevalid(MENU_PTR menu, int menuIndex, int subIndex)
{
	int ret = 1;
	if(menuIndex < 0 || subIndex < -1)
		ret = 0;
	//Check if optSel and subIndex exist	
	if(menuIndex > (menu->numopts)-1)
		ret = 0;
	if((subIndex != -1) && (subIndex > (menu->opt[menuIndex].numsubs)-1))
		ret = 0;
	return ret;
}
	
/*-------------------------------------------------------------------------
 *Add a shortcut character to a menu option or submenu option
 *optSel: Array index that selects the menu option. Starts at 0.
 *subMenu: Array index that selects submenu option. Starts at 0.
 *	If subMenu is -1, shortcut is added to menu option optSel instead.
 *If shortcutChar is already present in menu, returns failure
 *Returns 0 on failure or shortcutChar on success
 *-------------------------------------------------------------------------
 */
char menu_shortcut(MENU_PTR menu, int optSel, int subMenu, char shortcutChar)
{
	char ret = shortcutChar;
	//Check if arguments are valid
	if(!indexarevalid(menu, optSel, subMenu))
		ret = 0;
	//Check if shortcutChar is alpha
	if(!isal(shortcutChar))
		ret = 0;

	//Check if shortcut is assigned to another menu or submenu
	if(isredundant(menu, shortcutChar, optSel, subMenu))
		return 0;

	//Add shortcut to menu
	if(subMenu == -1)
		menu->opt[optSel].shortcut = shortcutChar;
	//Add shortcut to submenu
	else
		menu->opt[optSel].shortcuts[subMenu] = shortcutChar;

	return ret;
}


//TODO Everytime a key is pressed while currently on the last menu, 
//the first menu is highlited. FIX IT

/*----------------------------------------------------------------------
 *Displays the menu on the screen with item indexed by menuIndex highlighted
 *and displays window containing submenu indexed by subIndex.
 *If subIndex == -1 just display top itemns
 *Returns -1 on failure
 *----------------------------------------------------------------------
 */
static int menu_display(MENU_PTR menu, int menuIndex, int subIndex)	
{
	int i;
	int subMenuPos;	
	int opts = menu->numopts;
	WINDOW* wnd = menu->menuwnd;

	if(!indexarevalid(menu, menuIndex, subIndex))
		return 0;

	//Erase last display of menu and submenus from screen and Clear menu window
	touchwin(stdscr);
	refresh();
	wclear(wnd);

	//First display menu options with menuIndex option highlighted	
	for(i=0; i<opts; i++) //For each menu option...
	{
		char* menunam = menu->opt[i].menuname;
		char shortcut = menu->opt[i].shortcut;
		int oplen = strlen(menunam);
		int spaces = (menu->spacepads)/2;
		int j;

		if(i == menuIndex)
		{
			wattron(wnd, A_STANDOUT);
			subMenuPos = getcurx(wnd); //Used for drawing submenu if necessary
			//mvprintw(5,0,"subMenuPos: %d", subMenuPos); refresh();
		}
			
		//...diplay each character, underlining+bolding shortcuts	
		for(j=0; j<oplen; j++)
		{
			if(menunam[j] == shortcut)
			{
				wattron(wnd,A_BOLD | A_UNDERLINE);
				waddch(wnd,menunam[j]);
				wattroff(wnd,A_BOLD | A_UNDERLINE);
			}
			else
				waddch(wnd, menunam[j]);	
		}
		if(i == (opts-1))
			continue;
		//Print spaces and bars if enabled
		wattroff(wnd, A_STANDOUT);
		wprintw(wnd, "%*c", spaces, spaces==0 ? '\0' : ' ');
		wprintw(wnd, "%c", ((menu->flags & OPT_BAR)? '|':'\0'));
		wprintw(wnd, "%*c", spaces, spaces==0 ? '\0' : ' ');
			
	}

	//Now Display submenu selected by subIndex if it isnt -1
	if(subIndex != -1)
	{
		WINDOW *subpad, *subwnd;
		static int yStartPad = 0; //Keep track of the beginning of the pad
		int index;

		//Determine attributes for prefresh function
		int numLines, numCols, yStart, xStart, numsubs;
		yStart = menu->start_y + 2;
		xStart = menu->start_x + subMenuPos + 1; 
		numCols = MAX_LENGTH + 1;
		numsubs = numLines = menu->opt[menuIndex].numsubs;	

		if(numsubs > MAX_SUB_DISPLAY) 
		{
			numLines = MAX_SUB_DISPLAY;
			if((subIndex - yStartPad) > (MAX_SUB_DISPLAY - 1))
				yStartPad++;
			else if(subIndex < yStartPad)
				yStartPad--;	
		}
		if(subIndex == 0)
			yStartPad = 0;

		//Create submenu window and pad
		subwnd = newwin(numLines+2, numCols+2, yStart-1, xStart-1);
		subpad = newpad(numLines, numCols);
		if((subwnd == NULL) || (subpad == NULL))
		{
			menu_err |= ERROR_ALLOC_WINDOW;	
			return 0;	
		}
		box(subwnd, ACS_VLINE, ACS_HLINE);

		//TODO
		//Populate pad. THIS SHOULDNT HAVE TO BE DONE EVERY TIME MENUDISPLAY IS CALLED!!!!.
		//Populate it on init fuction, and the add BOLD UNDERLINE etc to the selected
		//line here.
		//TODO Getx pos could be used here to calculate length of largest submenu item.
		for(index=0; index < numsubs; index++)
		{
			if(index == subIndex) 
			{
				wattron(subpad,A_STANDOUT);
				mvwprintw(subpad, index, 0, "%s", menu->opt[menuIndex].subs[index]);
				wattroff(subpad,A_STANDOUT);
			}
			else
				mvwprintw(subpad, index, 0, "%s", menu->opt[menuIndex].subs[index]);
		}
		//TODO ADD '^' symbols if submenu has more option to scroll.
		//TODO ADD vertical bars to separate submenu items?

		wrefresh(subwnd);
		prefresh(subpad, yStartPad, 0, yStart, xStart, yStart+MAX_SUB_DISPLAY, xStart+numCols);
		delwin(subwnd);
		delwin(subpad);
	}	

	wrefresh(wnd);
	return 1;
}

void menu_show(MENU_PTR menu)
{
	if(!menu_display(menu, 0, -1))
		menu_err |= ERROR_SHOW_MENU;
}	

/*
static int isshortcut(int curX, int* curY)
{
	if(*curY ==
*/


int menu_action(MENU_PTR menu, int* menuChoice, int* subChoice)
{
	int curX, curY;
	chtype ch = 0;
	int shortcut = 0;
	int maxSubs, maxOpts = menu->numopts;
	curX = curY = 1;

	//Save keyboard input mode state and change it
	struct termios oldSettings, newSettings;
	tcgetattr(fileno(stdin), &oldSettings);	
	memcpy(&newSettings, &oldSettings, sizeof(struct termios));
	newSettings.c_lflag &= ~(ICANON | ECHO); //non-canonical, no-echo modes
	newSettings.c_cc[VMIN] = 1;
	newSettings.c_cc[VTIME] = 0;
	if(tcsetattr(fileno(stdin), TCSAFLUSH, &newSettings) != 0)
	{
		menu_err |= ERROR_CHANGE_IN;
		return -1;	
	}

	keypad(menu->menuwnd, TRUE);

	ch = getch();
	while((ch != ESCAPE_KEY) && (ch != '\n') && !shortcut)
	{
		maxSubs = menu->opt[curX-1].numsubs;
		switch(ch)
		{
			case KEY_LEFT:
				curY=1;
				curX = curX==1 ? maxOpts : curX-1; break;
			case KEY_RIGHT:
				curY=1;
				curX = (curX+1)>maxOpts ? 1 : curX+1; break;
			case KEY_UP:
				curY = curY==1 ? 1 : curY-1; break;
			case KEY_DOWN:
				curY = (curY+1) > maxSubs ? curY : curY+1; break;
			//TODO: Add shortcut access functionality
			//default:
			//	shortcut = isshortcut(curX, &curY);
		}	
		menu_display(menu, curX-1, curY-1);
		ch = getch();
		//Refresh main window to erase last menu and submenu from screen
		touchwin(stdscr);
		refresh();
	}
					
	//Restore keyboard input mode state
	if(tcsetattr(fileno(stdin), TCSANOW, &oldSettings) != 0)
		system("stty sane");
	keypad(menu->menuwnd, FALSE);

	if(ch == ESCAPE_KEY) //no choice selected
		return 0;
	
	*menuChoice = curX-1;
	*subChoice = curY-1;
	return 1;
}


void menu_end(MENU_PTR ptr)
{
	delwin(ptr->menuwnd);
	free(ptr);
}


void menu_perror(char* message)
{
	if(menu_err == 0)
		return;
	fprintf(stderr, "%s: ", message);
	if(menu_err & ERROR_TOO_WIDE)
		fprintf(stderr, "Menu width greater than screen width\n");
	if(menu_err & ERROR_ALLOC_WINDOW)
		fprintf(stderr, "Could not allocate memory for WINDOW structure\n");
	if(menu_err & ERROR_ALLOC_MENU)
		fprintf(stderr, "Could not allocate memory for MENU structure\n");
	if(menu_err & ERROR_MANY_SUBS)
		fprintf(stderr, "Too many sub menus. Maximum is %d\n", MAX_SUB);
	if(menu_err & ERROR_SHOW_MENU)
		fprintf(stderr, "Could not display menu\n");

	//Reset error
	menu_err = 0;
	
	return;
}




#ifndef MENU_H
#define MENU_H

//Constants used throughout code
#define ESCAPE_KEY	27	//ASCII for ESC key
#define MAX_OPTIONS	8	//Max number of option on menu
#define MAX_SUB		12	//Max number of sub-menu options
#define MAX_LENGTH	24	//Max number of characters for menu or sub-menu options
#define MAX_SUB_DISPLAY	8	//Jmax number of submenu items to diplay at once
#define NARROW_SPACES	2	//Used if SPACE_NARROW flag set
#define WIDE_SPACES	15	//Used if SPACE_WIDE flag set

//Constants for menu_init flags
#define TO_END		1 //Draw menu from x_start up to COLS
#define OPT_BAR		2 //Separate each option with a vertical bar
#define SPACE_NARROW	4 //Spacing between options. 
#define SPACE_WIDE	8

//Errors
#define ERROR_TOO_WIDE		1 //Menu is too large for screen
#define ERROR_ALLOC_WINDOW	2 //Problem allocating memory for WINDOW structure
#define ERROR_ALLOC_MENU	4 //Problem allocating memory for MENU structure
#define ERROR_MANY_SUBS		8 //Too many submenus
#define ERROR_SHOW_MENU		16 //Error in static function menu_display
#define ERROR_CHANGE_IN		32 //Problem trying to change input mode with termios


extern int menu_err;

//Used internally
typedef struct MENU {
	WINDOW* menuwnd;	
	int flags;
	short numopts;		//Number of menu options
	short spacepads;	//Number of spacing between options
	int start_x;		//Initial positions for menu on main screen
	int start_y;		
	//Array of structures containing each option, its submenus and its shortcuts
	struct option {
		char menuname[MAX_LENGTH];		//Name of option
		char shortcut; 				//Shortcut for this menu
		int numsubs;				//Number of sub menus
		char subs[MAX_SUB][MAX_LENGTH];		//Pointer to submenus for this option
		char shortcuts[MAX_SUB];		//Shortcuts characters for submenus
	} opt[MAX_OPTIONS];			//MAX_OPTIONS structures containing 1 menu option each
} MENU, *MENU_PTR;


MENU_PTR menu_init(int y_start, int x_start, char** menu_submenus_ptr, int flags);
char menu_shortcut(MENU_PTR, int menu, int subMenu, char shortcutChar);
void menu_show(MENU_PTR);
void menu_style(MENU_PTR, int flags);
int menu_action(MENU_PTR, int* menuChoice, int* subChoice);
void menu_perror(char* message);
void menu_end(MENU_PTR);

#endif



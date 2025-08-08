#include <cJSON.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <menu.h>
#include <string.h>
#include "parser.h"
#include "handler.h"
#define CTRLD 4
#define WIDTH 30
#define HEIGHT 10
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

int startx = 0;
int starty = 0;

struct stop
{
    char *name;
    char *id;
};

char *read_file(char *filename);

void func(char *name);

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);

char *create_menu(struct stop *stops, int n_choices, WINDOW *win, char *item_desc);

WINDOW *print_window_countdown(int starty, int startx, int height, int width, char *route, int timer1, int timer2);

void destroy_win(WINDOW *local_win);

//function to clear pointers and strings
void free_list(char **list, int n_choices);

int main()
{
    //locally get file
    char cwd[PATH_MAX];
    char fullpath[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        snprintf(fullpath, sizeof(fullpath), "%s/bus_sample.json", cwd);
    } else {
        perror("error getting cwd error");
        return 1;
    }
    char *data;
    cJSON *json;
    data = read_file("fullpath");
    json = cJSON_Parse(data);

    //for menu options
    char **choices;
    char **choices2;
    int n_choices, i;
    int c;
    char *route_id; //for route id
    char *stop; //for stop id
    char *response;//for api data
    struct stop *stops;
    struct stop *routes;
    int* times = NULL;

//free choice* and choice**
    ITEM **my_items;
    MENU *my_menu;
    WINDOW *my_menu_win;
    WINDOW *display_window;

    //init curses
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    init_pair(1, COLOR_GREEN, COLOR_BLACK);

    //create a window 
    my_menu_win = newwin(10, 40, 4, 4);
    keypad(my_menu_win, TRUE);

    //create menu
    n_choices = return_parent(json, &routes); //stores list items in routes
    route_id = create_menu(routes, n_choices, my_menu_win, NULL);
    for (int i = 0; i < n_choices; i++) {
    free(routes[i].name);
    }
    free(routes);


    
    n_choices = return_child(json, route_id, &stops);
    char *id;
    stop = create_menu(stops, n_choices, my_menu_win, id); 

    //send the request
    response = return_response(stop, route_id);
    
    //get time
    time_t current_time = time(&current_time);
    struct tm *lc_time = localtime(&current_time); 
    char ct[6];
    strftime(ct, sizeof(ct),  "%H:%M", lc_time);
    n_choices = return_ETA(response, ct, &times);
    wclear(my_menu_win);
    wrefresh(my_menu_win);
    endwin();


    int  temp, temp2;
    
    for ( i = 0; i < n_choices; i++)
	{
		temp = times[0];
		temp2 = times[1];
		//after this finishes, subtract a[0] from all other indicies, then shift list
		display_window = print_window_countdown(1, 1, 5, 35, route_id, times[0], times[1]);
		destroy_win(display_window);

		for (int j = 0; j < n_choices; j++)
		{
			//shift list to left by 1
			times[j] = times[j + 1];
		}
		//subtract time
		for (int j = 0; j < n_choices; j++)
		{
			times[j] = times[j] - temp;
		}
		//a[i] = a[i + 1];
		times[n_choices - 1] = temp;
		//a[N - 2] = temp2;
		refresh();
	}
    refresh();
    endwin();


    //for debugging
    printf("%d\n\n", n_choices);
    for (i = 0; i < n_choices; i++)
    {
        printf("%d\n", times[i]);
    }
    if (response)
    {
        printf("Response: %s\n", response);
        free(response);
    } else {
        fprintf(stderr, "Failed to get response.\n");
    }
    printf("%s : %s \n", route_id, stop);
    
    
    return 0;
}        

char *read_file(char *filename)
{
    //open file
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("[-] Error opening file.\n");
        return NULL;
    }
    //find size
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    //allocate size to a buffer
    char *buffer = malloc(size + 1);
    if (!buffer)
    {
        perror("[-] MEMORY allocation error");
        fclose(fp);
        return NULL;
    }
    //read data into buffer, null term, close file
    size_t fsize = fread(buffer, 1, size, fp);
    buffer[fsize] = '\0';//null term that hoe
    fclose(fp);

    return buffer; 
}

WINDOW *print_window_countdown(int starty, int startx, int height, int width, char *route, int timer1, int timer2)
{
	WINDOW *win = newwin(height, width, starty, startx);
	box(win, ACS_VLINE, 45);

	while (timer1 > 0)
	{
	mvwprintw(win, starty, startx, "%s...........................%02dm", route, timer1);
	//prevent negative printing
	if (timer2 > 0)
	{
	mvwprintw(win, starty + 2, startx, "%s...........................%02dm", route, timer2);
	}
	wrefresh(win);
	sleep(60);
	//if (timer1 == 0) break;
	timer1--;
	timer2--;
	}
	return win;
}

void destroy_win(WINDOW *local_win)
{

/* Show that box
*/
/* box(local_win, ' ', ' '); : This won't produce the desired
 * result of erasing the window. It will leave it's four corners
 * and so an ugly remnant of window.
 */
wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
/* The parameters taken are
 * 1. win: the window on which to operate
 * 2. ls: character to be used for the left side of the window
 * 3. rs: character to be used for the right side of the window
 * 4. ts: character to be used for the top side of the window
 * 5. bs: character to be used for the bottom side of the window
 * 6. tl: character to be used for the top left corner of the window
 * 7. tr: character to be used for the top right corner of the window
 * 8. bl: character to be used for the bottom left corner of the window
 * 9. br: character to be used for the bottom right corner of the window
 */
wrefresh(local_win);
werase(local_win);
delwin(local_win);
}


void func(char *name)
{	move(20, 0);
	clrtoeol();
	mvprintw(LINES - 3, 0, "Item selected is : %s", name);
}	

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color)
{	int length, x, y;
	float temp;

	if(win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if(startx != 0)
		x = startx;
	if(starty != 0)
		y = starty;
	if(width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length)/ 2;
	x = startx + (int)temp;
	wattron(win, color);
	mvwprintw(win, y, x, "%s", string);
	wattroff(win, color);
	refresh();
}

char *create_menu(struct stop *stops, int n_choices, WINDOW *win, char *item_desc)
{
    int i, c;
    MENU *my_menu;
    ITEM **my_items;
    char *name;

    //create items
    my_items =  (ITEM **)calloc(n_choices, sizeof(ITEM *));
    for (i = 0; i < n_choices; i++)
    {
        my_items[i] = new_item(stops[i].name, stops[i].id);
        set_item_userptr(my_items[i], func );
    }

    //creates menu
    my_menu = new_menu((ITEM **)my_items);

    //set menu to window
    set_menu_win(my_menu, win);
    set_menu_sub(my_menu, derwin(win, 6, 38, 3, 1));
    set_menu_format(my_menu, 5, 1);

    //set menu mark to the string
    set_menu_mark(my_menu, "> ");

    //print border and title/text
    box(win, ACS_VLINE, 45);
	print_in_middle(win, 1, 0, 40, "Menu", COLOR_PAIR(1));
	mvwaddch(win, 2, 0, ACS_LTEE);
    mvwhline(win, 2, 1, 45, 38);
    mvwaddch(win, 2, 39, ACS_RTEE);

    //post menu
    post_menu(my_menu);
    wrefresh(win);

    attron(COLOR_PAIR(1));
	mvprintw(LINES - 1, 0, "Arrow Keys to navigate (F1 to Exit)");
	attroff(COLOR_PAIR(1));
	refresh();

    bool exit_loop = false;
    while((c = wgetch(win)) != KEY_F(1))
	{       switch(c)
	        {	case KEY_DOWN:
				menu_driver(my_menu, REQ_DOWN_ITEM);
				break;
                case KEY_UP:
                    menu_driver(my_menu, REQ_UP_ITEM);
                    break;
                case KEY_NPAGE:
                    menu_driver(my_menu, REQ_SCR_DPAGE);
                    break;
                case KEY_PPAGE:
                    menu_driver(my_menu, REQ_SCR_UPAGE);
                    break;
                case 10: //enter
                {
                    ITEM *cur;
                    void(*p)(char *);

                    cur = current_item(my_menu);
                    //name = strdup(item_name(cur));
                    p = item_userptr(cur);
                    pos_menu_cursor(my_menu);
                    p((char *)item_name(cur));
                    if (item_desc == NULL)
                    {
                        name = strdup(item_name(cur));
                        break;
                    } else {
                        name = strdup(item_description(cur));
                    }

                    break;//exits case
                }
            break; //exit switch
		    }
    refresh();
    if (c == 10) // Enter pressed 
        break;//exit while
	}

    unpost_menu(my_menu);
    for(i = 0; i < n_choices; i++)
    {
        free_item(my_items[i]);
    }
    free_menu(my_menu);
    return name;
}

void free_list(char **list, int n_choices)
{
    for (int i = 0; i < n_choices; i++)
    {
        free(list[i]);//frees each string
    }
    free(list);//frees the array of pointers
}
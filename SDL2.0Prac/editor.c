#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <glib.h>
#include <jansson.h>
#include "entity.h"
#include "graphics.h"
#include "editor_save.h"
#include "editor_panel.h"
#include "editor_createroom.h"
#include "editor.h"

extern Game* game;
extern Editor_Panel *currentPanel;
extern GList* MainEditorPanels;
Workspace* workSpace;
SDL_Color BG_Color;

/*
This is where you can find all the drawing 
functions and parsing JSON for the Editor
*/

/*
@brief Creates the initial panels
*/
void Init_Editor()
{
	workSpace = CreateEditorWorkspace();
	CreateMainPanels();
	currentPanel = (Editor_Panel*)g_list_nth_data(MainEditorPanels,0);
	workSpace->tileSelector->SpriteSheet = (Sprite*)g_list_nth_data(workSpace->tileSelector->SpriteList,0);
	workSpace->map = NULL;
	atexit(FreeEveryThing);

}
/*
@brief Parses through JSON file to create Panels
*/
void CreateMainPanels()
{
	json_t *root;
	json_error_t err;
	json_t *panels,*parentPanel,*childPanel,*Labels,*childLabels,*Buttons,*childButtons,*TextBoxes,
		*childTextBoxes,*SpriteSheets,*childSpriteSheets, *Panels;
	Editor_Panel*  TilePanel,*MainPanel;
	Button* button;
	TextBox* textBox;
	SDL_Color color;
	Label* label; 
	GList *elem;
	char SpriteArray[10][255];
	SDL_Rect rect;
	int i,j,k,l,n,m;
	root = json_load_file("Editor.json",0,&err);
    if(!root)
	{
		fprintf(stderr,"error: on line %d : %s\n", err.line,err.text);
		exit(1);
	}
	panels = json_object_get(root,"Panels");
	for(i = 0;i < json_array_size(panels);++i)
	{
		parentPanel = json_array_get(panels,i);
		rect.x = json_number_value(json_object_get(parentPanel,"LocationX"));
		rect.y = json_number_value(json_object_get(parentPanel,"LocationY"));
		rect.w = json_number_value(json_object_get(parentPanel,"SizeX"));
		rect.h = json_number_value(json_object_get(parentPanel,"SizeY"));
		MainPanel = CreateEditorPanel(rect);
		MainPanel->visible = json_number_value(json_object_get(parentPanel,"Visible"));
		Panels = json_object_get(parentPanel,"Panels");
		//Assigning Child Panels
		for(j = 0;j< json_array_size(Panels);++j)
		{
			childPanel = json_array_get(Panels,j);
			if(!json_is_object(childPanel))
			{
				fprintf(stderr, "error: commit %d: Not an object\n", 0);
				json_decref(root);
				exit(1);
			}
			rect.x = json_number_value(json_object_get(childPanel,"LocationX"));
			rect.y = json_number_value(json_object_get(childPanel,"LocationY"));
			rect.w = json_number_value(json_object_get(childPanel,"SizeX"));
			rect.h = json_number_value(json_object_get(childPanel,"SizeY"));
			TilePanel = CreateEditorPanel(rect);
			TilePanel->siblingPanels = MainPanel->panels;
			if(strcmp(json_string_value(json_object_get(childPanel,"Name")),"TilePanel") == 0)
			{
				workSpace->tileSelector = 
					CreateTileSelector(CreateSDL_Rect(rect.x + 50,rect.y + 50,32,32));

			}
			TextBoxes = json_object_get(childPanel,"TextBoxes");
			for(k=0;k <json_array_size(TextBoxes);++k)
			{
				childTextBoxes = json_array_get(TextBoxes,k);
				if(!json_is_object(childTextBoxes))
				{
					fprintf(stderr, "error: commit %d: Not an object\n", 0);
					json_decref(root);
					exit(1);
				}
				//Create TextBoxes
				rect.x = json_number_value(json_object_get(childTextBoxes,"LocationX"))+ TilePanel->rect.x;
				rect.y = json_number_value(json_object_get(childTextBoxes,"LocationY"))+ TilePanel->rect.y;
				rect.w = json_number_value(json_object_get(childTextBoxes,"SizeX"));
				rect.h = json_number_value(json_object_get(childTextBoxes,"SizeY"));
				textBox = CreateEditorTextBox(rect);
				textBox->font = TTF_OpenFont(json_string_value(json_object_get(childTextBoxes,"Font")),json_number_value(json_object_get(childTextBoxes,"Size")));
				strcpy(textBox->name , (char*)json_string_value(json_object_get(childTextBoxes,"Name")));
				TilePanel->texts= g_list_append(TilePanel->texts,textBox);
			}
			//Create Labels
			Labels = json_object_get(childPanel,"Labels");
			for(k=0;k <json_array_size(Labels);++k)
			{
				childLabels = json_array_get(Labels,k);
				if(!json_is_object(childLabels))
				{
					fprintf(stderr, "error: commit %d: Not an object\n", 0);
					json_decref(root);
					exit(1);
				}
				rect.x = json_number_value(json_object_get(childLabels,"LocationX"))+ TilePanel->rect.x;
				rect.y = json_number_value(json_object_get(childLabels,"LocationY"))+ TilePanel->rect.y;
				rect.w = json_number_value(json_object_get(childLabels,"SizeX"));
				rect.h = json_number_value(json_object_get(childLabels,"SizeY"));
	
				label = CreateEditorLabel((char*)json_string_value(json_object_get(childLabels,"Text")),rect);
				strcpy(label->name,(char*)json_string_value(json_object_get(childLabels,"Name")));
				color.r = json_integer_value(json_object_get(childLabels,"ColorR"));
				color.g = json_integer_value(json_object_get(childLabels,"ColorG"));
				color.b = json_integer_value(json_object_get(childLabels,"ColorB"));

				label->color = color;

				label->parentPanel = TilePanel;
				TilePanel->labels = g_list_append(TilePanel->labels,label);
				if(strcmp(label->name,"CurrentRoom") == 0)
				{
					workSpace->RoomNumber = label;
				}
			}
			//Create Buttons
			Buttons = json_object_get(childPanel,"Buttons");
			for(k=0;k <json_array_size(Buttons);++k)
			{
				childButtons = json_array_get(Buttons,k);
				if(!json_is_object(childButtons))
				{
					fprintf(stderr, "error: commit %d: Not an object\n", 0);
					json_decref(root);
					exit(1);
				}
				rect.x = json_number_value(json_object_get(childButtons,"LocationX"))+ TilePanel->rect.x;
				rect.y = json_number_value(json_object_get(childButtons,"LocationY"))+ TilePanel->rect.y;
				rect.w = json_number_value(json_object_get(childButtons,"SizeX"));
				rect.h = json_number_value(json_object_get(childButtons,"SizeY"));
				button= CreateEditorButton(
					(char*)json_string_value(json_object_get(childButtons,"Sprite")),
					rect);
				strcpy(button->name,json_string_value(json_object_get(childButtons,"Name")));
				//Assign Function to Button
				if(strcmp((char*)json_string_value(json_object_get(childButtons,"Function")),"ChangeSpriteSheet") == 0)
					{
						button->function = &ChangeSpriteSheet;
						if(g_list_length(TilePanel->labels) !=0){
								for(n = 0; n < g_list_length(TilePanel->labels);++n)
								{
									elem = g_list_nth(TilePanel->labels,n);
									label = (Label*)elem->data;
									if(strcmp((char*)json_string_value(json_object_get(childButtons,"Label")),label->name)==0)
									{
										button->data = label->text;
										break;
									}
								}
							}
					}
				else if(strcmp((char*)json_string_value(json_object_get(childButtons,"Function")),"LoadEditorMapPanel") == 0)
				{
					button->function = &LoadEditorMapPanel;
					button->data = workSpace->map;
				}
				else if(strcmp((char*)json_string_value(json_object_get(childButtons,"Function")),"SaveMap") == 0)
				{
					button->function = &SaveMap;
				}
				else if (strcmp((char*)json_string_value(json_object_get(childButtons,"Function")),"IncrementActiveLayer") == 0)
				{
					button->function = &IncrementActiveLayer;
					if(g_list_length(TilePanel->labels) !=0){
						for(n = 0; n < g_list_length(TilePanel->labels);++n)
						{
							elem = g_list_nth(TilePanel->labels,n);
							label = (Label*)elem->data;
							if(strcmp((char*)json_string_value(json_object_get(childButtons,"Label")),label->name)==0)
							{
								button->data = label->text;
								break;
							}
						}
					}	

				}else if (strcmp((char*)json_string_value(json_object_get(childButtons,"Function")),"DecrementActiveLayer") == 0)
				{
					button->function = &DecrementActiveLayer;
					if(g_list_length(TilePanel->labels) !=0){
						for(n = 0; n < g_list_length(TilePanel->labels);++n)
						{
							elem = g_list_nth(TilePanel->labels,n);
							label = (Label*)elem->data;
							if(strcmp((char*)json_string_value(json_object_get(childButtons,"Label")),label->name)==0)
							{
								button->data = label->text;
								break;
							}
						}
					}		

				}else if (strcmp((char*)json_string_value(json_object_get(childButtons,"Function")),"CreateRoom") == 0)
				{
					button->function = &LoadRoomCreatePanel;
				}
				TilePanel->buttons = g_list_append(TilePanel->buttons,button);

			}
			MainPanel->panels = g_list_append(MainPanel->panels,TilePanel);
	}
	}
	MainEditorPanels= g_list_append(MainEditorPanels,MainPanel);
	json_decref(root);
}
/*
@brief Draws buttons of panels
@param Pointer to GList of Button structs
*/
void DrawEditorButtons(GList* buttons)
{
	BG_Color.r = 100;BG_Color.b = 0;BG_Color.g = 0;
	GList *elem;
	Button *ref;
	SDL_Rect src;
	int n;
		if(g_list_length(buttons) != 0){
		for(n = 0; n < g_list_length(buttons);++n)
		{
			elem = g_list_nth(buttons,n);
			ref = (Button*)elem->data;
			src.x = 0;
			src.y = 0;
			src.w =ref->sprite->w;
			src.h =ref->sprite->h;
			SDL_RenderCopyEx(GetRenderer(), ref->sprite->image,&src,&ref->rect,0,0,SDL_FLIP_NONE);
		}

	}
}
/*
@brief Draws Labels of panels
@param Pointer to GList of Label structs
*/
void DrawEditorLabels(GList *labels)
{
	BG_Color.r = 100;BG_Color.b = 0;BG_Color.g = 0;
	GList *elem,*elem2;
	Label* ref;
	int n,m;
 	if(g_list_length(labels) != 0){
		for(n = 0; n < g_list_length(labels);++n)
		{
			elem = g_list_nth(labels,n);
			ref = (Label*)elem->data;
			if(ref->rect.y < ref->parentPanel->rect.y + ref->parentPanel->rect.h  &&
				ref->rect.y > ref->parentPanel->rect.y)
			{
				RenderFont(ref->text,ref->rect,game->font,&ref->color);
			}
		}
	}
}
/*
@brief Draws TextBoxes of panels
@param Pointer to GList of TextBox structs
*/
void DrawEditorTextBoxes(GList* texts)
  {
	GList *elem;
	TextBox* ref;
	int n,m;
 	if(g_list_length(texts) != 0){
		for(n = 0; n < g_list_length(texts);++n)
		{
			elem = g_list_nth(texts,n);
			ref = (TextBox*)elem->data;
			SDL_SetRenderDrawColor(GetRenderer(), BG_Color.r, BG_Color.g, BG_Color.b, BG_Color.a); 
			SDL_RenderDrawRect(GetRenderer(), &ref->rect);
			SDL_SetRenderDrawColor(GetRenderer(), BG_Color.g, BG_Color.g, BG_Color.b, BG_Color.a);
			RenderFont(ref->value,ref->rect,game->font,&GetColor("white"));
		}
	}
}
/*
@brief Draws the currentSprite Tile from the tileSelector
*/
void DrawCurrentSpriteTile()
{

	SDL_Rect dest,src;
	if(workSpace->tileSelector->visible){
		DrawEditorButtons(workSpace->tileSelector->buttons);
		RenderFont(workSpace->tileSelector->label->text,workSpace->tileSelector->label->rect,game->font,&workSpace->tileSelector->label->color);
		if(workSpace->tileSelector->SpriteSheet != NULL){
			src.x = (workSpace->tileSelector->frameNum-1)%workSpace->tileSelector->SpriteSheet->fpl * workSpace->tileSelector->SpriteSheet->w;
			src.y = (workSpace->tileSelector->frameNum-1)/workSpace->tileSelector->SpriteSheet->fpl * workSpace->tileSelector->SpriteSheet->h;
			src.w =workSpace->tileSelector->SpriteSheet->w;
			src.h =workSpace->tileSelector->SpriteSheet->h;
			dest.x = workSpace->tileSelector->position.x;
			dest.y = workSpace->tileSelector->position.y;
			dest.w = workSpace->tileSelector->SpriteSheet->w;
			dest.h = workSpace->tileSelector->SpriteSheet->h;
			SDL_RenderCopyEx(GetRenderer(),workSpace->tileSelector->SpriteSheet->image,&src,&dest,0,0,SDL_FLIP_NONE);
		}
	}
}

/*
@brief Calls all Draw functions on Panels
@param Pointer to GList of Panel structs
*/
void DrawEditorPanels(GList* panels)
  {
	GList *elem,*elem2;
	Editor_Panel* ref,*ref2;
	int n,m;
 	if(g_list_length(panels) != 0){
		for(n = g_list_length(panels)-1; n >=0;--n)
		{
			elem = g_list_nth(panels,n);
			ref = (Editor_Panel*)elem->data;
			if(!ref->visible)
				continue;
			DrawCurrentSpriteTile();
			SDL_SetRenderDrawColor(GetRenderer(), 255, 0, 0, 0); 
			SDL_RenderDrawRect(GetRenderer(), &ref->rect);
			DrawEditorLabels(ref->labels);
			DrawEditorButtons(ref->buttons);
			DrawEditorTextBoxes(ref->texts);
			if(g_list_length(ref->panels) != 0){
				for(m = 0; m < g_list_length(ref->panels);++m)
				{
					elem2 = g_list_nth(ref->panels,m);
					ref2 = (Editor_Panel*)elem2->data;
					SDL_SetRenderDrawColor(GetRenderer(), BG_Color.r, BG_Color.g, BG_Color.b, BG_Color.a); 
					SDL_RenderDrawRect(GetRenderer(), &ref2->rect);
					SDL_SetRenderDrawColor(GetRenderer(), BG_Color.g, BG_Color.g, BG_Color.b, BG_Color.a);
					DrawEditorLabels(ref2->labels);
					DrawEditorButtons(ref2->buttons);
					DrawEditorTextBoxes(ref2->texts);

				}
			//g_list_foreach(ref->panels,(GFunc)DrawEditorPanels,ref->panels);
			
		}
	}
	}
}
/*
@brief Draws the map onto the workspaces buffer than draws the buffer to the screen
*/
void DrawWorkspace()
{
	int x,y;
	GList *elem;
	Sprite* ref;
	int frame = 0;
	SDL_Rect tileSelect;
	SDL_Rect worldSize;
	SDL_Rect roomSelect;
	if(workSpace->map != NULL)
	{
		worldSize.x = worldSize.y = 0;
		worldSize.w = 25 * workSpace->map->tileW;
		worldSize.h = 19 * workSpace->map->tileH;

		SDL_SetRenderTarget(GetRenderer(),workSpace->buffer);
		SDL_RenderClear(GetRenderer());
		DrawMap(1,0,0,workSpace->buffer);
		DrawMap(2,0,0,workSpace->buffer);
		DrawMap(3,0,0,workSpace->buffer);
		//Draws Rectangle
		SDL_GetMouseState(&x,&y);
		x += workSpace->areaToDraw.x;
		y += workSpace->areaToDraw.y;
		if(workSpace->activeLayer == 1)
			frame = GetTile(workSpace->map->data,x/32,y/32);
		else if(workSpace->activeLayer == 2)
			frame = GetTile(workSpace->map->data2,x/32,y/32);
		else if(workSpace->activeLayer == 3)
			frame = GetTile(workSpace->map->data3,x/32,y/32);
		elem = g_list_nth(workSpace->tileSelector->SpriteList,0);
		ref = (Sprite*) elem->data;
		tileSelect.x = (x/32)*32 + (game->camera->x);
		tileSelect.y = (y/32)*32 + (game->camera->y);
		tileSelect.w = workSpace->map->tileW;
		tileSelect.h = workSpace->map->tileH;

		SDL_SetRenderTarget(GetRenderer(),workSpace->buffer);
		//Draw Room Rectangle
		if(workSpace->currentRoom != NULL){
			roomSelect.x= workSpace->currentRoom->boundary.x+ (game->camera->x);
			roomSelect.y= workSpace->currentRoom->boundary.y+ (game->camera->y);
			roomSelect.h= workSpace->currentRoom->boundary.h;
			roomSelect.w= workSpace->currentRoom->boundary.w;
			SDL_SetRenderDrawColor(GetRenderer(), 255, 0, 0, 0); 
			SDL_RenderDrawRect(GetRenderer(), &roomSelect);
		}
		if(workSpace->mode == Add){
			if(frame == 0)
				SDL_SetRenderDrawColor(GetRenderer(), 255, 0, 0, 0); 
			else
				SDL_SetRenderDrawColor(GetRenderer(), 0, 255, 0, 0); 
			SDL_RenderDrawRect(GetRenderer(), &tileSelect);
		}
		else if(workSpace->mode == Remove)
		{
			SDL_SetRenderDrawColor(GetRenderer(), 0, 0, 0, 0); 
			SDL_RenderFillRect(GetRenderer(), &tileSelect);
		}
		SDL_SetRenderTarget(GetRenderer(),NULL);
		printf("%d\n",frame);
		//Renders buffer to screen
		SDL_RenderCopy(GetRenderer(),workSpace->buffer,&workSpace->areaToDraw,&worldSize);
	}
}
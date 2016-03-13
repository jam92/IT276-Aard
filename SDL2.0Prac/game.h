#ifndef __GAME_H
#define __GAME_H


Vec2D GetCameraPosition();
Vec2D GetCameraSize();
Vec2D SetCameraPosition;
void SetCameraSize(Vec2D size);
SDL_Rect* GetCamera();	


void DrawTitle();
void DrawGame();
void Init();
void Loop();
void SetCamera(SDL_Rect &camera,SDL_Rect *box);
void UpdateBox();
SDL_Rect *InitBox();
int (*gameState)();
int Title();
int Game();
void SetUpMap(Map *map,FILE *file);

#endif

/**
 * gametest3d
 * @license The MIT License (MIT)
 *   @copyright Copyright (c) 2015 EngineerOfLies
 *    Permission is hereby granted, free of charge, to any person obtaining a copy
 *    of this software and associated documentation files (the "Software"), to deal
 *    in the Software without restriction, including without limitation the rights
 *    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *    copies of the Software, and to permit persons to whom the Software is
 *    furnished to do so, subject to the following conditions:
 *    The above copyright notice and this permission notice shall be included in all
 *    copies or substantial portions of the Software.
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *    SOFTWARE.
 */
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL_mixer.h>
#include <SDL_audio.h>
#include <SDL.h>
#include <SDL_mouse.h>
#include "string.h"
#include "mgl_callback.h"
#include "simple_logger.h"
#include "graphics3d.h"
#include "shader.h"
#include "obj.h"
#include "vector.h"
#include "sprite.h"
#include "entity.h"
#include "space.h"
#include "collision.h"
#include "body.h"

void set_camera(Vec3D position, Vec3D rotation);
void pollEvents();
void updateBounds();
void touch_callback(void *data, void *context);
void drawHealth(Vec3D Offset);
void drawMana(Vec3D Offset);
Entity *newPlayer(Vec3D position,const char *name);
Entity *newFloor(Vec3D position,const char *name);
Entity *newWall(Vec3D position,const char *name);
Entity *newObstacle(Vec3D position,const char *name);
Entity *newEnemy(Vec3D position,const char *name, int health, int attack);
Entity *newArrow(Vec3D position, const char *name, int powerAttack); //0 for no, 1 for yes
char bGameLoopRunning = 1;
SDL_Event e;
Vec3D cameraPosition = {0,0,1};
Vec3D cameraRotation = {90,0,0};
Vec3D cameraPlayerOffset = {0,-3,2};
Vec3D camOffset, finalMove;
Vec3D newCameraPosition, newArrowPos,arrowFinal, arrowOffset = {0,3,1};
Entity *player, *arenaWall, *arenaFloor, *arenaObstacle[10], 
	*arenaEnemy1, *arenaEnemy2, *arenaEnemy3, *Arrow;
Sprite *enemy1, *enemy2, *enemy3;
Space *space;
Mix_Chunk *explosionEffect, *bowEffect, *deathEffect;
GLfloat xAxis, yAxis;
int xMouse, yMouse; 
int xhigh = 720, xlow = 0;
int yhigh = 135, ylow = 45;
int playerRotating = 0;
int playerMoving = 0;
float maxXVeloc = .5, maxYVeloc = .5, maxZVeloc = .5;
float minXVeloc = -.5, minYVeloc = -.5, minZVeloc = -.5;
Vec3D direction, enemy1Direction, enemy2Direction, enemy3Direction;
Sprite *healthbar, *manabar;

int main(int argc, char *argv[])
{
	int j = 0, k = -50, i=0, d = -30, c = 10;
    GLuint vao;
	Vec3D newCamRotation;
	int enemy1Radius;
    float r = 0;
    GLuint triangleBufferObject;
    const float triangleVertices[] = {
        0.0f, 0.5f, 0.0f, 1.0f,
        0.5f, -0.366f, 0.0f, 1.0f,
        -0.5f, -0.366f, 0.0f, 1.0f,
        //next part contains vertex colors
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f  
    }; //we love you vertices!
    
    init_logger("gametest3d.log");
    if (graphics3d_init(1024,768,1,"gametest3d",33) != 0)
    {
        return -1;
    }
    model_init();
    obj_init();
	entity_init(255);

	player = newPlayer(vec3d(10,-5,2),"Player1");
	arenaWall = newWall(vec3d(0,0,0),"Wall");
	arenaFloor = newFloor(vec3d(0,0,0),"Floor");
	for(j = 0; j < 10; j++)
	{
		arenaObstacle[j] = newObstacle(vec3d(k,k,4),"Obstacle");
		k += 10;
	}
		
	arenaEnemy1 = newEnemy(vec3d(d,c,2.5),"Enemy1", 30, 1);
	d += 5;
	c -= 5;
	arenaEnemy2 = newEnemy(vec3d(d,c,2.5),"Enemy2", 60, 1);
	d += 5;
	c -= 5;
	arenaEnemy3 = newEnemy(vec3d(d,c,2.5),"Enemy3", 90, 1);
    healthbar = LoadSprite("textures/health_bar.png",606,46);
	manabar = LoadSprite("textures/mana_bar.png",259,26);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao); //make our vertex array object, we need it to restore state we set after binding it. Re-binding reloads the state associated with it.
    
    glGenBuffers(1, &triangleBufferObject); //create the buffer
    glBindBuffer(GL_ARRAY_BUFFER, triangleBufferObject); //we're "using" this one now
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW); //formatting the data for the buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind any buffers
	SDL_ShowCursor(SDL_DISABLE); //disable the mouse cursor on the game screen
	//SDL_SetRelativeMouseMode(SDL_TRUE); //Set the relative mouse mode to lock mouse to center of screen
	space = space_new();
    space_set_steps(space,100);
    
    space_add_body(space,&player->body);
	for(j = 0; j < 10; j++)
		space_add_body(space,&arenaObstacle[j]->body);
	space_add_body(space,&arenaEnemy1->body);
	space_add_body(space,&arenaEnemy2->body);
	space_add_body(space,&arenaEnemy3->body);

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	explosionEffect = Mix_LoadWAV("sounds/explosion.wav");
	bowEffect = Mix_LoadWAV("sounds/bow.wav");
	deathEffect = Mix_LoadWAV("sounds/death.wav");
	SDL_WarpMouseInWindow(NULL, 1024/2,768/2);

    while (bGameLoopRunning) //Main Loop
    {
		entity_think_all();
        for (i = 0; i < 100;i++)
        {
            space_do_step(space);
        }
		
		pollEvents();
		SDL_WarpMouseInWindow(NULL, 1024/2,768/2);

        graphics3d_frame_begin();

		if (Arrow != NULL)
		{
			vec3d_cpy(Arrow->position,Arrow->body.position);				//Update mesh position to body for the arrow
			if (Arrow->body.position.y > 100 || Arrow->body.position.x > 100 || Arrow->body.position.y < -100 || Arrow->body.position.x < -100)
			{ 
					slog("Arrow destroyed");
					space_remove_body(space, &Arrow->body);
					entity_free(Arrow);
					memset(&Arrow,0,sizeof(Entity));
					Mix_PlayChannel(-1,explosionEffect, 0);
			}
		}
		//===============================
		// PLAYER VELOCITY BOUNDING
		//===============================
		if(player->body.velocity.x > 0)										// slow down on x, y, and z
			player->body.velocity.x -= .008;
		else if(player->body.velocity.x < 0)								// slow down on x, y, and z
			player->body.velocity.x += .008;
		else 
			player->body.velocity.x = 0;

		if(player->body.velocity.y > 0)
			player->body.velocity.y -= .008;
		else if(player->body.velocity.y < 0)
			player->body.velocity.y += .008;
		else 
			player->body.velocity.y = 0;

		if(player->body.velocity.z > 0)
			player->body.velocity.z -= .064;
		//if(player->body.velocity.z >= 2)
			//player->body.velocity.z -= .064;

		if(player->body.position.z <= 2)									//Prevent floor clipping
			player->body.position.z = 2;
		//===============================
		// END PLAYER VELOCITY BOUNDING
		//===============================


		//===============================
		// ENEMY MOVEMENT UPDATING
		//===============================
		if(arenaEnemy1 != NULL)
		{
			enemy1Direction.x = player->position.x - arenaEnemy1->position.x;	//This could be done as vector, just cleaner to read
			enemy1Direction.y = player->position.y - arenaEnemy1->position.y;	//this way
			vec3d_cpy(arenaEnemy1->position, arenaEnemy1->body.position);
			arenaEnemy1->body.velocity.x = enemy1Direction.x * .01;
			arenaEnemy1->body.velocity.y = enemy1Direction.y * .01;
		}
		if(arenaEnemy2 != NULL)
		{
			enemy2Direction.x = player->position.x - arenaEnemy2->position.x;
			enemy2Direction.y = player->position.y - arenaEnemy2->position.y;
			vec3d_cpy(arenaEnemy2->position, arenaEnemy2->body.position);
			arenaEnemy2->body.velocity.x = enemy2Direction.x * .018;
			arenaEnemy2->body.velocity.y = enemy2Direction.y * .018;
		}
		if(arenaEnemy3 != NULL)
		{
			enemy3Direction.x = player->position.x - arenaEnemy3->position.x;
			enemy3Direction.y = player->position.y - arenaEnemy3->position.y;
			vec3d_cpy(arenaEnemy3->position, arenaEnemy3->body.position);
			arenaEnemy3->body.velocity.x = enemy3Direction.x * .015;
			arenaEnemy3->body.velocity.y = enemy3Direction.y * .015;
		}
		//===============================
		// END ENEMY MOVEMENT UPDATING
		//===============================


        glPushMatrix();
		if(playerRotating > 0)
		{
			if(player->rotation.y + xAxis >= xhigh)
				player->rotation.y = xhigh;
			else if(player->rotation.y + xAxis <= xlow)
				player->rotation.y = xlow;
			else
				player->rotation.y += xAxis;
			playerRotating = 0;
			//slog("the value is %f", xAxis);
		}
		glPopMatrix();
		glPushMatrix();
		vec3d_add(newCameraPosition,player->position,vec3d(sin(player->rotation.y * DEGTORAD),-cos(player->rotation.y * DEGTORAD),2));	//Update camera position 
		vec3d_updateCam(newCamRotation, player->rotation);					//Fix messed up rotations between player/camera
		set_camera(newCameraPosition,newCamRotation);						//Apply camera to new snap point
		vec3d_cpy(player->position,player->body.position);					//update mesh position to body for player
		
		entity_draw_all();
        glPopMatrix();
		drawHealth(vec3d(-.0001,0.07,0));
		drawMana(vec3d(-.001,.15,0));
        /* drawing code above here! */
        graphics3d_next_frame();
		if(player->health <= 0)
		{
			Mix_Quit();
			return 0;
		}
    } 
	Mix_Quit();
    return 0;
}

void touch_callback(void *data, void *context)
{
    Entity *me,*other;
    Body *obody;
    if ((!data)||(!context))return;
    me = (Entity *)data;
    obody = (Body *)context;
    if (entity_is_entity(obody->touch.data))
    {
        other = (Entity *)obody->touch.data;
        //slog("other is: %s",other->name);
		//slog("me is: %s",me->name);
		//slog("name: %s",Arrow->name);
		if(strcmp(me->name, "Enemy1")==0 && strcmp(other->name, "Player1")==0)
		{
			other->health -= me->attack;
			slog("me is: %s",me->name);
			slog("other is: %s",other->name);
			slog("Player health is: %i", other->health);
		}

		if(strcmp(me->name, "Enemy2")==0 && strcmp(other->name, "Player1")==0)
		{
			other->health -= me->attack;
			slog("me is: %s",me->name);
			slog("other is: %s",other->name);
			slog("Player health is: %i", other->health);
		}

		if(strcmp(me->name, "Enemy3")==0 && strcmp(other->name, "Player1")==0)
		{
			other->health -= me->attack;
			slog("me is: %s",me->name);
			slog("other is: %s",other->name);
			slog("Player health is: %i", other->health);
		}
		
		if(strcmp(me->name, "Arrow")==0)
		{
			if(strcmp(other->name,"Obstacle")==0)
			{
				// do nothing and destroy self
				slog("Arrow destroyed");
				space_remove_body(space, &Arrow->body);
				entity_free(Arrow);
				memset(&Arrow,0,sizeof(Entity));
				Mix_PlayChannel(-1,explosionEffect, 0);
				
			}

			if(strcmp(other->name,"Enemy1")==0 && arenaEnemy1 != NULL)
			{
				
				if(Arrow->usesOMana == 1)
					arenaEnemy1->health -= 50;
				else
					arenaEnemy1->health -= 35;
				
				space_remove_body(space, &Arrow->body);
				entity_free(Arrow);
				memset(&Arrow,0,sizeof(Entity));
				Mix_PlayChannel(-1,explosionEffect, 0);
				slog("Arrow destroyed1");
				slog("Enemy1 HP: %i",arenaEnemy1->health);
				
				if (arenaEnemy1->health <= 0)
				{
					space_remove_body(space, &arenaEnemy1->body);
					entity_free(arenaEnemy1);
					arenaEnemy1 = NULL;
					Mix_PlayChannel(-1,deathEffect, 0);
				}
				
			}

			if(strcmp(other->name,"Enemy2")==0)
			{
				
				if(Arrow->usesOMana == 1)
					arenaEnemy2->health -= 50;
				else
					arenaEnemy2->health -= 35;
				
				space_remove_body(space, &Arrow->body);
				entity_free(Arrow);
				memset(&Arrow,0,sizeof(Entity));
				Mix_PlayChannel(-1,explosionEffect, 0);
				slog("Arrow destroyed2");
				slog("Enemy2 HP: %i",arenaEnemy2->health);
				
				if (arenaEnemy2->health <= 0)
				{
					space_remove_body(space, &arenaEnemy2->body);
					entity_free(arenaEnemy2);
					arenaEnemy2 = NULL;
					Mix_PlayChannel(-1,deathEffect, 0);
				}
				
			}

			if(strcmp(other->name,"Enemy3")==0)
			{
				
				if(me->usesOMana == 1)
					arenaEnemy3->health -= 50;
				else
					arenaEnemy3->health -= 35;
					
				space_remove_body(space, &Arrow->body);
				entity_free(Arrow);
				memset(&Arrow,0,sizeof(Entity));
				Mix_PlayChannel(-1,explosionEffect, 0);
				slog("Arrow destroyed3");
				slog("Enemy3 HP: %i",arenaEnemy3->health);
				
				if (arenaEnemy3->health <= 0)
				{
					space_remove_body(space, &arenaEnemy3->body);
					entity_free(arenaEnemy3);
					arenaEnemy3 = NULL;
					Mix_PlayChannel(-1,deathEffect, 0);
				}
				
			}

		}
    }
    
} //Collision checks

Entity *newArrow(Vec3D position, const char *name, int powerAttack)
{
	Entity * ent;
    char buffer[255];
    int i;
    ent = entity_new();
    if (!ent)
    {
        return NULL;
    }
    ent->objModel = obj_load("models/arrow.obj");
    ent->texture = LoadSprite("models/arrow_text.png",1024,1024);
	ent->usesOMana = powerAttack;
	strcpy(ent->name, name);
	vec3d_cpy(ent->position,position);
    vec3d_cpy(ent->body.position,ent->position);
    cube_set(ent->body.bounds,0,0,0,.5,.5,.5);
    ent->rotation.x = 90;
	ent->rotation.z = 0;
    sprintf(ent->name,"%s",name);
    //ent->think = think;
    mgl_callback_set(&ent->body.touch,touch_callback,ent);
    return ent;
}

Entity *newPlayer(Vec3D position,const char *name)
{
    Entity * ent;
    char buffer[255];
    int i;
    ent = entity_new();
    if (!ent)
    {
        return NULL;
    }
	ent->uid = 1;
    ent->objModel = obj_load("models/cube.obj");
    ent->texture = LoadSprite("models/cube_text.png",1024,1024);
	ent->health = 100;
	ent->offensiveMana = 101;
	strcpy(ent->name, name);
	vec3d_cpy(ent->position,position);
    vec3d_cpy(ent->body.position,ent->position);
    cube_set(ent->body.bounds,-1,-1,-1,2,2,2);
    ent->rotation.x = 90;
	ent->rotation.z = 0;
    sprintf(ent->name,"%s",name);
    //ent->think = think;
    mgl_callback_set(&ent->body.touch,touch_callback,ent);
    return ent;
}

Entity *newEnemy(Vec3D position,const char *name, int health, int attack)
{
	Entity * ent;
    char buffer[255];
    int i, alpha = 255;
    ent = entity_new();
	
    if (!ent)
    {
        return NULL;
    }
	if(strcmp(name, "Enemy1")==0)
		ent->texture = LoadSprite("models/enemy1_text.png",1024,1024);
	else if(strcmp(name, "Enemy2")==0)
		ent->texture = LoadSprite("models/enemy2_text.png",1024,1024);
	else
		ent->texture = LoadSprite("models/enemy3_text.png",1024,1024);

    ent->objModel = obj_load("models/cube.obj");
	ent->health = health;
	ent->attack = attack;
	strcpy(ent->name, name);
	vec3d_cpy(ent->position,position);
    vec3d_cpy(ent->body.position,ent->position);
    cube_set(ent->body.bounds,-1,-1,-1,2,2,2);
    ent->rotation.x = 90;
	ent->rotation.z = 0;
    sprintf(ent->name,"%s",name);
    //ent->think = think;
    mgl_callback_set(&ent->body.touch,touch_callback,ent);
    return ent;
}

Entity *newFloor(Vec3D position,const char *name)
{
    Entity * ent;
    char buffer[255];
    int i;
    ent = entity_new();
    if (!ent)
    {
        return NULL;
    }
	ent->uid = 2;
    ent->objModel = obj_load("models/arena_floor.obj");
    ent->texture = LoadSprite("models/arena_floor_text.png",1024,1024);
	vec3d_cpy(ent->position,position);
    vec3d_cpy(ent->body.position,position);
	strcpy(ent->name, name);
    //cube_set(ent->body.bounds,-80,-80,1,2,2,2);
    ent->rotation.x = 90;
	ent->rotation.y = 90;
    sprintf(ent->name,"%s",name);
    //ent->think = think;
    mgl_callback_set(&ent->body.touch,touch_callback,ent);
    return ent;
}

Entity *newWall(Vec3D position,const char *name)
{
    Entity * ent;
    char buffer[255];
    int i;
    ent = entity_new();
    if (!ent)
    {
        return NULL;
    }
	ent->uid = 3;
    ent->objModel = obj_load("models/arena_wall.obj");
    ent->texture = LoadSprite("models/arena_wall_text.png",1024,1024);
	vec3d_cpy(ent->position,position);
    vec3d_cpy(ent->body.position,position);
	strcpy(ent->name, name);
    //cube_set(ent->body.bounds,0,0,0,0,0,0); Dont neeed for this
    ent->rotation.x = 90;
	ent->rotation.y = 90;
    sprintf(ent->name,"%s",name);
    //ent->think = think;
    mgl_callback_set(&ent->body.touch,touch_callback,ent);
    return ent;
}

Entity *newObstacle(Vec3D position,const char *name)
{
    Entity * ent;
    char buffer[255];
	ent = entity_new();
	if (!ent)
	{
		return NULL;
	}
	ent->objModel = obj_load("models/arena_cube.obj");
	ent->texture = LoadSprite("models/arena_cube_text.png",1024,1024);
	vec3d_cpy(ent->position,position);
	vec3d_cpy(ent->body.position,ent->position);
	cube_set(ent->body.bounds,-2,-2,-2,5,5,5);
	strcpy(ent->name, name);
	ent->rotation.x = 90;
	ent->rotation.y = 90;
	sprintf(ent->name,"%s",name);
	mgl_callback_set(&ent->body.touch,touch_callback,ent);
	return ent;
}

void set_camera(Vec3D position, Vec3D rotation)
{
    glRotatef(-rotation.x, 1.0f, 0.0f, 0.0f);
    glRotatef(-rotation.y, 0.0f, 1.0f, 0.0f);
    glRotatef(-rotation.z, 0.0f, 0.0f, 1.0f);
    glTranslatef(-position.x,
                 -position.y,
                 -position.z);
}

void pollEvents()
{
	float xrad, yrad;
	while ( SDL_PollEvent(&e) ) 
        {
            if (e.type == SDL_QUIT)
            {
                bGameLoopRunning = 0;
            }
			
			if (e.type == SDL_MOUSEMOTION)
			{
				playerRotating = 1;
				SDL_GetMouseState(&xMouse, &yMouse);
				
				xAxis = (512 - xMouse) * .625;
				yAxis = (384 - yMouse) * .625;
				
				xrad = (512 - xMouse) / 512.0 * 3.14 / 2.0;
				yrad = (384 - yMouse) / 384.0 * 3.14 / 2.0;

				//direction.x = cos(yrad) * sin(xrad);
				//direction.y = sin(yrad);
				//direction.z = 0;

				/*
				if(player->rotation.y + xAxis >= xhigh)
					player->rotation.y = xhigh;
				else if(player->rotation.y + xAxis <= xlow)
					player->rotation.y = xlow;
				else
					player->rotation.y += xAxis;

				if(player->rotation.x + yAxis >= yhigh)
					player->rotation.x = yhigh;
				else if(player->rotation.x + yAxis <= ylow)
					player->rotation.x = ylow;
				else
					player->rotation.x += yAxis;
					*/
			}
            else if (e.type == SDL_KEYDOWN)
            {
				switch(e.key.keysym.sym)
				{
					case SDLK_w:
					{
						//vec3d_cpy(direction, vec3d(-sin(player->rotation.y * DEGTORAD),cos(player->rotation.y * DEGTORAD),0));
						//vec3d_scale(finalMove, direction, 1);
						player->body.velocity = vec3d(-sin(player->rotation.y * DEGTORAD)*.5,cos(player->rotation.y * DEGTORAD)*.5,0);
						//player->body.velocity.x = 20 * direction.x;
						//player->body.velocity.y = 20 * direction.y;
						//player->body.velocity.z = 20 * direction.z;
						//if(player->body.velocity.y > maxYVeloc)
							//player->body.velocity.y = maxYVeloc;
						break;
					}
					case SDLK_a:
					{
						player->body.velocity = vec3d(-cos(player->rotation.y * DEGTORAD)*.5,-sin(player->rotation.y * DEGTORAD)*.5,0);
						//player->body.velocity.x += -.1;
						//if(player->body.velocity.x < minXVeloc)
							//player->body.velocity.x = minXVeloc;
						break;
					}
					case SDLK_s:
					{
						player->body.velocity = vec3d(sin(player->rotation.y * DEGTORAD)*.5,-cos(player->rotation.y * DEGTORAD)*.5,0);
						//player->body.velocity.y += -.1;
						//if(player->body.velocity.y < minYVeloc)
							//player->body.velocity.y = minYVeloc;
						break;
					}
					case SDLK_d:
					{
						player->body.velocity = vec3d(cos(player->rotation.y * DEGTORAD)*.5,sin(player->rotation.y * DEGTORAD)*.5,0);
						//player->body.velocity.x += .1;
						//if(player->body.velocity.x > maxXVeloc)
							//player->body.velocity.x = maxXVeloc;
						break;
					}
					//case SDLK_SPACE:
					//{
						//player->body.velocity.z += 1;
						//break;
					//}
					case SDLK_ESCAPE:
					{
						bGameLoopRunning = 0;
						break;
					}
				}
            }
			if (e.button.button == SDL_BUTTON_LEFT)
			{
				if(Arrow != NULL)
				{
					//Already in use
					continue;
				}
				else
				{
					//vec3d(-sin(player->rotation.y * DEGTORAD),cos(player->rotation.y * DEGTORAD),0);
					vec3d_add(newArrowPos,player->body.position,arrowOffset);
					//vec3d_add(arrowFinal, newArrowPos, ); 
					Arrow = newArrow(newArrowPos,"Arrow", 0);
					space_add_body(space,&Arrow->body);
					//vec3d_cpy(Arrow->body.position, Arrow->position);
					Arrow->body.velocity = vec3d(-sin(player->rotation.y * DEGTORAD),cos(player->rotation.y * DEGTORAD),0);
					Mix_PlayChannel(-1,bowEffect, 0);
				}
			}
			if (e.button.button == SDL_BUTTON_RIGHT)
			{
				if(Arrow != NULL)
				{
					//Already in use
					continue;
				}
				else
				{
					if(player->offensiveMana >= 25)
					{
						vec3d_add(newArrowPos,player->body.position,arrowOffset);
						Arrow = newArrow(newArrowPos,"Arrow", 1);
						space_add_body(space,&Arrow->body);
						Arrow->body.velocity = vec3d(-sin(player->rotation.y * DEGTORAD),cos(player->rotation.y * DEGTORAD),0);
						player->offensiveMana -= 25;
						if(player->offensiveMana < 0)
							player->offensiveMana = 0;
						Mix_PlayChannel(-1,bowEffect, 0);
					}
				}
			}
	}
}

void drawHealth(Vec3D offset)
{
	Vec3D hpverts[4];
	Vec2D hpuVs[4];
	int h = player->health;
	float hscale;

	hscale = (h / 100.0) - 1.0;

	//bottom left vert
	hpverts[0].x=-1;
	hpverts[0].y=.9f;
	hpverts[0].z=1;
	//top left vert
	hpverts[1].x=-1;
	hpverts[1].y=1;
	hpverts[1].z=1;
	//top right vert
	hpverts[2].x=-.01f + hscale;
	hpverts[2].y=1;
	hpverts[2].z=1;
	//bottom right vert
	hpverts[3].x=-.01f + hscale;
	hpverts[3].y=0.9f;
	hpverts[3].z=1;

	//bottom left UV
	hpuVs[0].x= offset.x;
	hpuVs[0].y= offset.y;
	//top left UV
	hpuVs[1].x= offset.x;
	hpuVs[1].y= 1 - offset.y;
	//bottom right UV
	hpuVs[2].x=.5;
	hpuVs[2].y= offset.y;
	//top right UV
	hpuVs[3].x=.5;
	hpuVs[3].y=1;
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(-1.0f,1,-1,1);
    glEnable(GL_BLEND);
    glColorMaterial(GL_FRONT,GL_DIFFUSE);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glTranslatef(-offset.x,-offset.y,-offset.z);
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D,healthbar->texture);
	
	glBegin(GL_TRIANGLES);

	glTexCoord2f(hpuVs[0].x,hpuVs[0].y);
	glVertex3f(hpverts[0].x,hpverts[0].y,hpverts[0].z);
	
	glTexCoord2f(hpuVs[2].x,hpuVs[2].y);
	glVertex3f(hpverts[2].x,hpverts[2].y,hpverts[2].z);

	glTexCoord2f(hpuVs[1].x,hpuVs[1].y);
	glVertex3f(hpverts[1].x,hpverts[1].y,hpverts[1].z);

	glTexCoord2f(hpuVs[0].x,hpuVs[0].y);
	glVertex3f(hpverts[0].x,hpverts[0].y,hpverts[0].z);

	glTexCoord2f(hpuVs[3].x,hpuVs[3].y);
	glVertex3f(hpverts[3].x,hpverts[3].y,hpverts[3].z);

	glTexCoord2f(hpuVs[2].x,hpuVs[2].y);
	glVertex3f(hpverts[2].x,hpverts[2].y,hpverts[2].z);
	
	glEnd();
    glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
    glPopMatrix();
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	
}

void drawMana(Vec3D offset)
{
	Vec3D manaverts[4];
	Vec2D manauVs[4];
	int m = player->offensiveMana;
	float mscale;

	mscale = (m / 100.0) - 1.0;
	//bottom left vert
	manaverts[0].x=-1;
	manaverts[0].y=.95f;
	manaverts[0].z=1;
	//top left vert
	manaverts[1].x=-1;
	manaverts[1].y=1;
	manaverts[1].z=1;
	//top right vert
	manaverts[2].x=-.1f + mscale;
	manaverts[2].y=1;
	manaverts[2].z=1;
	//bottom right vert
	manaverts[3].x=-.1f + mscale;
	manaverts[3].y=0.95f;
	manaverts[3].z=1;

	//bottom left UV
	manauVs[0].x= offset.x;
	manauVs[0].y= offset.y;
	//top left UV
	manauVs[1].x= offset.x;
	manauVs[1].y= 1 - offset.y;
	//bottom right UV
	manauVs[2].x=.5;
	manauVs[2].y= offset.y;
	//top right UV
	manauVs[3].x=.5;
	manauVs[3].y=1;
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(-1.0f,1,-1,1);
    glEnable(GL_BLEND);
    glColorMaterial(GL_FRONT,GL_DIFFUSE);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glTranslatef(-offset.x,-offset.y,-offset.z);
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D,manabar->texture);
	
	glBegin(GL_TRIANGLES);

	glTexCoord2f(manauVs[0].x,manauVs[0].y);
	glVertex3f(manaverts[0].x,manaverts[0].y,manaverts[0].z);
	
	glTexCoord2f(manauVs[2].x,manauVs[2].y);
	glVertex3f(manaverts[2].x,manaverts[2].y,manaverts[2].z);

	glTexCoord2f(manauVs[1].x,manauVs[1].y);
	glVertex3f(manaverts[1].x,manaverts[1].y,manaverts[1].z);

	glTexCoord2f(manauVs[0].x,manauVs[0].y);
	glVertex3f(manaverts[0].x,manaverts[0].y,manaverts[0].z);

	glTexCoord2f(manauVs[3].x,manauVs[3].y);
	glVertex3f(manaverts[3].x,manaverts[3].y,manaverts[3].z);

	glTexCoord2f(manauVs[2].x,manauVs[2].y);
	glVertex3f(manaverts[2].x,manaverts[2].y,manaverts[2].z);
	
	glEnd();
    glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
    glPopMatrix();
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
}

/*eol@eof*/
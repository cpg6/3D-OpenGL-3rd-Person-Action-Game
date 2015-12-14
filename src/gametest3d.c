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
void drawHealthBar();
void playerMove();
void playerRotate();
Entity *newPlayer(Vec3D position,const char *name);
Entity *newFloor(Vec3D position,const char *name);
Entity *newWall(Vec3D position,const char *name);
Entity *newObstacle(Vec3D position,const char *name);
Entity *newEnemy(Vec3D position,const char *name, int health, int attack, Sprite *texture);
Entity *newArrow(Vec3D position, const char *name, int powerAttack); //0 for no, 1 for yes
char bGameLoopRunning = 1;
SDL_Event e;
Vec3D cameraPosition = {0,0,1};
Vec3D cameraRotation = {90,0,0};
Vec3D cameraPlayerOffset = {0,-3,2};
Vec3D newCameraPosition, newArrowPos, arrowOffset = {0,3,1};
Entity *player, *arenaWall, *arenaFloor, *arenaObstacle[10], 
	*arenaEnemy1, *arenaEnemy2, *arenaEnemy3, *Arrow;
Sprite *enemy1, *enemy2, *enemy3;
Space *space;
Mix_Chunk *explosionEffect, *bowEffect, *deathEffect;
float red = 125, green = 250, blue = 150;
GLfloat xAxis, yAxis;
int xMouse, yMouse; 
int xhigh = 720, xlow = 0;
int yhigh = 135, ylow = 45;
int playerRotating = 0;
int playerMoving = 0;
float maxXVeloc = .5, maxYVeloc = .5, maxZVeloc = .5;
float minXVeloc = -.5, minYVeloc = -.5, minZVeloc = -.5;
Vec3D direction;
//bowEffect = Mix_LoadWAV("sounds/bow.wav");
//explosionEffect = Mix_LoadWAV("sounds/explosion.wav");
//deathEffect = Mix_LoadWAV("sounds/death.wav");

int main(int argc, char *argv[])
{
	int j = 0, k = -50, i=0, d = -30, c = 10;
    GLuint vao;
	Vec3D newCamRotation;
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

	enemy1 = LoadSprite("models/enemy1_text.png",1024,1024);
	enemy2 = LoadSprite("models/enemy2_text.png",1024,1024);
	enemy3 = LoadSprite("models/enemy3_text.png",1024,1024);
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao); //make our vertex array object, we need it to restore state we set after binding it. Re-binding reloads the state associated with it.
    
    glGenBuffers(1, &triangleBufferObject); //create the buffer
    glBindBuffer(GL_ARRAY_BUFFER, triangleBufferObject); //we're "using" this one now
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW); //formatting the data for the buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind any buffers
   // SDL_ShowCursor(SDL_DISABLE); //disable the mouse cursor on the game screen
	//SDL_SetRelativeMouseMode(SDL_TRUE); //Set the relative mouse mode to lock mouse to center of screen

	player = newPlayer(vec3d(10,-5,2),"Player1");
	arenaWall = newWall(vec3d(0,0,0),"Wall");
	arenaFloor = newFloor(vec3d(0,0,0),"Floor");
	for(j = 0; j < 10; j++)
	{
		arenaObstacle[j] = newObstacle(vec3d(k,k,4),"Obstacle");
		k += 10;
	}
		
	arenaEnemy1 = newEnemy(vec3d(d,c,2.0),"Enemy1", 30, 1, enemy1);
	d += 5;
	c -= 5;
	arenaEnemy2 = newEnemy(vec3d(d,c,2.0),"Enemy2", 60, 1, enemy2);
	d += 5;
	c -= 5;
	arenaEnemy3 = newEnemy(vec3d(d,c,2.0),"Enemy3", 90, 1, enemy3);

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
		
		//TODO: Insert flag check to call playerRotate and playerMove for after frame begin

		if (Arrow != NULL)
			vec3d_cpy(Arrow->position,Arrow->body.position);				//Update mesh position to body for the arrow
		if (Arrow!=NULL && Arrow->body.position.y > 100)
		{
			slog("Arrow destroyed");
			space_remove_body(space, &Arrow->body);
			entity_free(Arrow);
			memset(&Arrow,0,sizeof(Entity));
			Mix_PlayChannel(-1,explosionEffect, 0);
		}
		
		if(player->body.velocity.x > 0)										// slow down on x, y, and z
			player->body.velocity.x -= .028;
		if(player->body.velocity.x < 0)										// slow down on x, y, and z
			player->body.velocity.x += .028;
		if(player->body.velocity.y > 0)
			player->body.velocity.y -= .028;
		if(player->body.velocity.y < 0)
			player->body.velocity.y += .028;
		if(player->body.velocity.z > 0)
			player->body.velocity.z -= .064;
		if(player->body.position.z > 2)
			player->body.position.z -= .064;
		if(player->body.position.z <= 2)									//Prevent floor clipping
				player->body.position.z = 2;
		
		


        glPushMatrix();
		if(playerRotating > 0)
		{
			//glPushMatrix();
			//glRotatef(xAxis, 0.0f,1.0f,0.0f);
			if(player->rotation.y + xAxis >= xhigh)
				player->rotation.y = xhigh;
			else if(player->rotation.y + xAxis <= xlow)
				player->rotation.y = xlow;
			else
				player->rotation.y += xAxis;

			//entity_draw(player);
			//glPopMatrix();
			playerRotating = 0;
			slog("the value is %f", xAxis);
		}
		glPopMatrix();
		glPushMatrix();
		vec3d_add(newCameraPosition,player->position,vec3d(sin(player->rotation.y * DEGTORAD),-cos(player->rotation.y * DEGTORAD),2));	//Update camera position 
		vec3d_updateCam(newCamRotation, player->rotation);					//Fix messed up rotations between player/camera
		set_camera(newCameraPosition,newCamRotation);						//Apply camera to new snap point
		vec3d_cpy(player->position,player->body.position);					//update mesh position to body for player
		entity_draw_all();
        glPopMatrix();

        /* drawing code above here! */
        graphics3d_next_frame();
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

			if(strcmp(other->name,"Enemy1")==0)
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
					Mix_PlayChannel(-1,deathEffect, 0);
					//memset(&arenaEnemy1,0,sizeof(Entity));
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
					Mix_PlayChannel(-1,deathEffect, 0);
					//memset(&arenaEnemy2,0,sizeof(Entity));
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
					Mix_PlayChannel(-1,deathEffect, 0);
					//memset(&arenaEnemy3,0,sizeof(Entity));
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
	ent->rotation.z = 360;
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
	ent->offensiveMana = 50;
	strcpy(ent->name, name);
	vec3d_cpy(ent->position,position);
    vec3d_cpy(ent->body.position,ent->position);
    cube_set(ent->body.bounds,-1,-1,-1,2,2,2);
    ent->rotation.x = 90;
	ent->rotation.z = 360;
    sprintf(ent->name,"%s",name);
    //ent->think = think;
    mgl_callback_set(&ent->body.touch,touch_callback,ent);
    return ent;
}

Entity *newEnemy(Vec3D position,const char *name, int health, int attack, Sprite *texture)
{
	Entity * ent;
    char buffer[255];
    int i, alpha = 255;
    ent = entity_new();
	
    if (!ent)
    {
        return NULL;
    }
    ent->objModel = obj_load("models/cube.obj");
    ent->texture = texture;
	ent->health = health;
	ent->attack = attack;
	strcpy(ent->name, name);
	vec3d_cpy(ent->position,position);
    vec3d_cpy(ent->body.position,ent->position);
    cube_set(ent->body.bounds,-1,-1,-1,2,2,2);
    ent->rotation.x = 90;
	ent->rotation.z = 360;
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
	//playerRotating = 1;
	//playerMoving = 0;
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

				direction.x = cos(yrad) * sin(xrad);
				direction.y = sin(yrad);
				direction.z = 0;

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
			//vec3d(-sin(player->rotation.z * DEGTORAD),cos(player->rotation.z * DEGTORAD),0);
            else if (e.type == SDL_KEYDOWN)
            {
				switch(e.key.keysym.sym)
				{
					case SDLK_w:
					{
						player->body.velocity = vec3d(-sin(player->rotation.y * DEGTORAD),cos(player->rotation.y * DEGTORAD),0);
						//player->body.velocity.x = 20 * direction.x;
						//player->body.velocity.y = 20 * direction.y;
						//player->body.velocity.z = 20 * direction.z;
						//if(player->body.velocity.y > maxYVeloc)
							//player->body.velocity.y = maxYVeloc;
						break;
					}
					case SDLK_a:
					{
						player->body.velocity = vec3d(-cos(player->rotation.y * DEGTORAD),-sin(player->rotation.y * DEGTORAD),0);
						//player->body.velocity.x += -.1;
						//if(player->body.velocity.x < minXVeloc)
							//player->body.velocity.x = minXVeloc;
						break;
					}
					case SDLK_s:
					{
						player->body.velocity = vec3d(sin(player->rotation.y * DEGTORAD),-cos(player->rotation.y * DEGTORAD),0);
						//player->body.velocity.y += -.1;
						//if(player->body.velocity.y < minYVeloc)
							//player->body.velocity.y = minYVeloc;
						break;
					}
					case SDLK_d:
					{
						player->body.velocity = vec3d(cos(player->rotation.y * DEGTORAD),sin(player->rotation.y * DEGTORAD),0);
						//player->body.velocity.x += .1;
						//if(player->body.velocity.x > maxXVeloc)
							//player->body.velocity.x = maxXVeloc;
						break;
					}
					case SDLK_SPACE:
					{
						player->body.velocity.z = maxZVeloc;
						break;
					}
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
					Arrow = newArrow(newArrowPos,"Arrow", 0);
					space_add_body(space,&Arrow->body);
					Arrow->body.velocity.y = 3;
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
					if(player->offensiveMana >= 10)
					{
						vec3d_add(newArrowPos,player->body.position,arrowOffset);
						Arrow = newArrow(newArrowPos,"Arrow", 1);
						space_add_body(space,&Arrow->body);
						Arrow->body.velocity.y = 3;
						player->offensiveMana -= 10;
						if(player->offensiveMana < 0)
							player->offensiveMana = 0;
						Mix_PlayChannel(-1,bowEffect, 0);
					}
				}
			}
	}
}

void drawHealthBar()
{

}

void playerRotate()
{
	// need the angle for rotation


}

void playerMove()
{

}

/*eol@eof*/
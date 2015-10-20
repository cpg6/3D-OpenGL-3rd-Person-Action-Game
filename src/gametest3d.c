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
Entity *newPlayer(Vec3D position,const char *name);
Entity *newFloor(Vec3D position,const char *name);
Entity *newWall(Vec3D position,const char *name);
Entity *newObstacle(Vec3D position,const char *name);
void touch_callback(void *data, void *context);
char bGameLoopRunning = 1;
SDL_Event e;
Vec3D cameraPosition = {0,0,1};
Vec3D cameraRotation = {90,0,0};
Vec3D cameraPlayerOffset = {0,-3,2};
Vec3D newCameraPosition;
Entity *player, *arenaWall, *arenaFloor, *arenaObstacle[10];

int xMouse, yMouse, xAxis, yAxis;
int xhigh = 720, xlow = 0;
int yhigh = 135, ylow = 45;

float maxXVeloc = .5, maxYVeloc = .5, maxZVeloc = .5;
float minXVeloc = -.5, minYVeloc = -.5, minZVeloc = -.5;

int main(int argc, char *argv[])
{
	int j = 0, k = -50, i=0;
    GLuint vao;
	Space *space;
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
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao); //make our vertex array object, we need it to restore state we set after binding it. Re-binding reloads the state associated with it.
    
    glGenBuffers(1, &triangleBufferObject); //create the buffer
    glBindBuffer(GL_ARRAY_BUFFER, triangleBufferObject); //we're "using" this one now
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW); //formatting the data for the buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind any buffers
    
	player = newPlayer(vec3d(10,-5,2),"Player1");
	arenaWall = newWall(vec3d(0,0,0),"Wall");
	arenaFloor = newFloor(vec3d(0,0,0),"Floor");
	for(j = 0; j < 10; j++)
	{
		arenaObstacle[j] = newObstacle(vec3d(k,k,4),"Obstacle");
		k += 10;
	}

	space = space_new();
    space_set_steps(space,100);
    
    space_add_body(space,&player->body);
	for(j = 0; j < 10; j++)
		space_add_body(space,&arenaObstacle[j]->body);

    while (bGameLoopRunning) //Main Loop
    {
		entity_think_all();
        for (i = 0; i < 100;i++)
        {
            space_do_step(space);
        }
		pollEvents();
        graphics3d_frame_begin();

        vec3d_add(newCameraPosition,player->position,cameraPlayerOffset);	//Update camera position 
		set_camera(newCameraPosition,player->rotation);						//Apply camera to new snap point

		vec3d_cpy(player->position,player->body.position);					//update mesh position to body position
		if(player->body.velocity.x > 0)										// slow down on x, y, and z
			player->body.velocity.x -= .008;
		if(player->body.velocity.x < 0)										// slow down on x, y, and z
			player->body.velocity.x += .008;
		if(player->body.velocity.y > 0)
			player->body.velocity.y -= .008;
		if(player->body.velocity.y < 0)
			player->body.velocity.y += .008;
		if(player->body.velocity.z > 0)
			player->body.velocity.z -= .064;
		if(player->body.position.z > 2)
			player->body.position.z -= .064;
		if(player->body.position.z <= 2)									//Prevent floor clipping
				player->body.position.z = 2;
        glPushMatrix();

		entity_draw_all();
		
        glPopMatrix();
        /* drawing code above here! */
        graphics3d_next_frame();
    } 
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
        slog("%s is ",other->name);
    }
    slog("touching me.... touching youuuuuuuu");
}

void pollEvents()
{
	while ( SDL_PollEvent(&e) ) 
        {
            if (e.type == SDL_QUIT)
            {
                bGameLoopRunning = 0;
            }
			
			if (e.type == SDL_MOUSEMOTION)
			{

				SDL_GetMouseState(&xMouse, &yMouse);

				xAxis = ((1024/2) - xMouse) * .00625;
				yAxis = ((768/2) - yMouse) * .00625;

				if(player->rotation.z + xAxis >= xhigh)
					player->rotation.z = xhigh;
				else if(player->rotation.z + xAxis <= xlow)
					player->rotation.z = xlow;
				else
					player->rotation.z += xAxis;

				if(player->rotation.x + yAxis >= yhigh)
					player->rotation.x = yhigh;
				else if(player->rotation.x + yAxis <= ylow)
					player->rotation.x = ylow;
				else
					player->rotation.x += yAxis;
			}
			
            else if (e.type == SDL_KEYDOWN)
            {
				switch(e.key.keysym.sym)
				{
					case SDLK_w:
					{
						player->body.velocity.y += .1;
						if(player->body.velocity.y > maxYVeloc)
							player->body.velocity.y = maxYVeloc;
						break;
					}
					case SDLK_a:
					{
						player->body.velocity.x += -.1;
						if(player->body.velocity.x < minXVeloc)
							player->body.velocity.x = minXVeloc;
						break;
					}
					case SDLK_s:
					{
						player->body.velocity.y += -.1;
						if(player->body.velocity.y < minYVeloc)
							player->body.velocity.y = minYVeloc;
						break;
					}
					case SDLK_d:
					{
						player->body.velocity.x += .1;
						if(player->body.velocity.x > maxXVeloc)
							player->body.velocity.x = maxXVeloc;
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
        }
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

/*eol@eof*/
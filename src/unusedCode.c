/*
	//GAMETEST3D.C

	//slog("touching me.... touching youuuuuuuu");

	Obj *objPlayer,*objFloor,*objWall, *objCube;  //World/player objects
    Sprite *textPlayer,*textFloor,*textWall,*textCube;   //World/player Sprites

	//Load all objects and textures into memory
    objPlayer = obj_load("models/cube.obj"); 
    textPlayer = LoadSprite("models/cube_text.png",1024,1024);

    objFloor = obj_load("models/arena_floor.obj");
    textFloor = LoadSprite("models/arena_floor_text.png",1024,1024);

	objWall = obj_load("models/arena_wall.obj");
	textWall = LoadSprite("models/arena_wall_text.png",1024,1024);

	objCube = obj_load("models/arena_cube.obj");
	textCube = LoadSprite("models/arena_cube_text.png",1024,1024);

			//if(player->body.bounds.r < arenaWall->body.bounds.r) //Will always be true, need to fix
		//{
			//printf("Inside the wall");
		//}
		//if(cube_cube_intersection(player->body.bounds, arenaFloor->body.bounds))
		//{
		//	printf("Hit the floor");
		//}

	for(j = 0; j < 10; j++)
		{
			if(cube_cube_intersection(cubePlayer, cubeObstacle[j]))
			{
				printf("Hit an Obstacle");
			}
		}

			if(player->position.z > 2)					//Junk jump handle method atm
				player->position.z -= .05;
	
		obj_draw(				//level floor
            objFloor,		//level floor
            vec3d(0,0,0),	//position
            vec3d(90,90,0), //rotation
            vec3d(1,1,1),	//scale
            vec4d(1,1,1,1), //color
            textFloor		//texture sprite
        );
    
		obj_draw(				//level wall
            objWall,		//level wall
            vec3d(0,0,0),	//position
            vec3d(90,90,0), //rotation
            vec3d(1,1,1),	//scale
            vec4d(1,1,1,1), //color
            textWall		//texture sprite
        );
		k = -50;
		
		{
		obj_draw(				//level obstacles
            objCube,			//level cubes
            vec3d(k,k,2),		//position
            vec3d(90,90,0),		//rotation
            vec3d(.25,.25,.25),	//scale
            vec4d(1,1,1,1),		//color
            textCube			//texture sprite
        );
		k += 10;
		}
        k = -50;
		
		
        obj_draw(				//player
            objPlayer,
            vec3d(10,-2,2),		//position
            vec3d(90,0,0),
            vec3d(0.5,0.5,0.5),
            vec4d(1,1,1,1),
            textPlayer
        );
		

        //if (r > 360)r -= 360;

		vec3d_add(
                        player->position,
                        player->position,
                        vec3d(
                            sin(player->rotation.z * DEGTORAD),
                            -cos(player->rotation.z * DEGTORAD),
                            0
                        ));

		else if (e.key.keysym.sym == SDLK_w)
                {
                    vec3d_add(
                        cameraPosition,
                        cameraPosition,
                        vec3d(
                            -sin(cameraRotation.z * DEGTORAD),
                            cos(cameraRotation.z * DEGTORAD),
                            0
                        ));
                }
                else if (e.key.keysym.sym == SDLK_s)
                {
                    vec3d_add(
                        cameraPosition,
                        cameraPosition,
                        vec3d(
                            sin(cameraRotation.z * DEGTORAD),
                            -cos(cameraRotation.z * DEGTORAD),
                            0
                        ));
                }
                else if (e.key.keysym.sym == SDLK_d)
                {
                    vec3d_add(
                        cameraPosition,
                        cameraPosition,
                        vec3d(
                            cos(cameraRotation.z * DEGTORAD),
                            sin(cameraRotation.z * DEGTORAD),
                            0
                        ));
                }
                else if (e.key.keysym.sym == SDLK_a)
                {
                    vec3d_add(
                        cameraPosition,
                        cameraPosition,
                        vec3d(
                            -cos(cameraRotation.z * DEGTORAD),
                            -sin(cameraRotation.z * DEGTORAD),
                            0
                        ));
	else if (e.key.keysym.sym == SDLK_x)
                {
                    player->body.velocity.z = -.2;
                }
				else if (e.key.keysym.sym == SDLK_w)
                {
                    
                }
				if (e.key.keysym.sym == SDLK_ESCAPE)
                {
                    
                }
                else if (e.key.keysym.sym == SDLK_SPACE)
                {
                    
                }
                
                else if (e.key.keysym.sym == SDLK_s)
                {
                    
                }
                else if (e.key.keysym.sym == SDLK_d)
                {
                    
                }
                else if (e.key.keysym.sym == SDLK_a)
                {
                    
                }
*/
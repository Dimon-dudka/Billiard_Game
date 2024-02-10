# Billiard Game
The implementation of the game logic of the game "billiards" based on the existing framework.

## Building 
Build by launching .sln file for Microsoft Visual Studio.

## Description
The goal of the project is to implement game logic based on a ready-made framework. The following features have been implemented for this purpose:
- physics of the player's ball moving towards the mouse after accumulating impact energy
- physics of balls colliding with the walls of the game table
- physics of balls colliding with each other
- removing balls when they hit the hole
- restart the game after player's ball hit the hole

## Game rules 
To give movement to the main ball, you need to hold down the mouse and release it. While the mouse is pressed, the impact force with which the ball will fly will increase. The ball will fly in the direction of the computer mouse. It will be possible to hit the ball again only after all the balls have stopped.  
When the main ball hits the hole, the game restarts.  
To exit the game, you need to press the escape. To restart the game, press the space.

## Technology stack:
- C++ 17
- Visual Studio 2022
- Git
- OpenGL


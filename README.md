# Voronoi Demo

## Code info
Shape class: where all logic for implementing the voronoi cells is. Voronoi animation is enabled with public function generateVoronoi()

Animation is set using the setAnimationFunction() with a function pointer that takes one float and returns a float

An animation function takes in distance and outputs an offset into the animation. So using distance squared means at further distances the animation timeline will be stretched (ie slows down further away). Similarly, doing something like the squareroot of the distance will compress the animation timeline at further distances (ie speeds up further away). Positive and negative values will cause the animation to either radiate outward from the master point or inward toward the master point.

Terrain class: extends shape class and allows for shape to be created from a heightmap image. None of the voronoi logic is implemented here

Shaders: voronoi draw function expects shader to have an extra S matrix for local transformations. Should be P*V*M*S*vertPos in vertex shader.


## Controls:
W - move forward

S - move backward

A - move left

D - move right

M - toggle mouse mode between locked and unlocked

Z - render shape outlines only

ESC - exit program


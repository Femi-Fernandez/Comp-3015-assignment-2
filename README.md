

## How to run
1. Open the "Build" folder.
2. Run the executable titled "Project_Template"

### To view the code
1. Open the "Comp-3015-assignment-2" folder.
2. Open the "Project_Template.sln" file in Visual Studio.

# Shader discussion
This project features 3 shading techniques: a particle system, volumetric shadows and edge detection. The particle system is featured by itself, while the volumetric shadows and edge detection are featured together. 

### Particle system
The particle system is handled inside a Vertex shader.  to start, it stores the particles maximum lifetime, the location the particles should appear from, as well as the particles starting size and end size. 
In the method that updates the position of the particle, the Euler method is used to approximate the position of the particle based on how long the particle has been alive, the particles velocity, and its start position.
In the method that renders the particle, if the particle is alive its size will increase based on how long its been alive and its max life time. 
These methods are run based on what pass the main script is on. 

### Shadows / Edge Detection
Shadows and edge detection are primarily handled in the same shaders. the shadows utilise two shaders, one to calculate the shadow's position, and one to render the models that will be casting the shadow. This is done in a Geometry shader. If a face is pointing toward the light source, it checks each adjacent triangle, and if one isn't it sets the edge as a silhouette edge.

In the second shader, variables for the light, textures, and material properties are setup, as well as the edge detection variables.  A method to apply shading of the object using the Blinn-Phong method is here so the object has shading.  The edge detection checks each pixel to see if there is a big enough colour difference between it and any adjacent pixels, and if there is, set the colour of the pixel to the edge detection colour.

In the main method, two if statements are setup to act as Passes. The first pass runs the shading script, while also applying the normal map, and the second script sets the textures of the objects in the scene, as well as running the edge detection script. 

## Main Script (scenebasic_uniform.cpp)
Scenebasic_uniform.cpp handles setting the variables of the shaders, as well as loading and rendering the models. 
#### start
To start, it loads the ground, walls and bike objects to be used later in the script. 
It sets up the frame and vertex buffers for the shadows and particle system, as well as loading all the textures and setting the unchanging variables in each shader. 
#### update
In update, if the scene is running, it updates a variable, deltaT, which is used to keep track of the amount of time the program has been running for.  An Int, LightingType, is set, which updates whenever the light makes a half-rotation around the scene. If it gets too large, it resets back to its initial value. the edge detection intensity is also set here, as a small script was written that increases and decreases the alpha of the edge detection lines. 
#### render
In render, two if statements control what rendering method is being used.  If the render type is 1, it renders the particles, if it is 2 it renders the shadows and edge detection.
#### rendering particle system
the particle scene renders the ground and the bike before rendering the particles. 
The particle renderer first sets the active textures as the smoke texture, and then updates the position of each of the particles being rendered. It then renders each particle. 
#### rendering shadows / edge detection

Rendering the shadows and edge detection is done in 3 separate methods. The first method sets the lights position and renders the shading for the objects. 
the second sets up the stencils for the shadows, and the last draws the shadows and edge detection lines in. 
#### drawScene / renderConc
DrawScene and renderConc almost do the same thing. they both draw in the ground, walls and bike, as well as set the light intensity and material values. 

## Known issues
Upon running the program, the particles initially all spawn at once at 0,0,0. They begin to render correctly once the scene they are in reloads. 

## video link

https://youtu.be/SuPpAILVgL4
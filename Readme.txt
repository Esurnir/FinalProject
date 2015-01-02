Library Requirement : FreeGlut, Glew, GLM, GLI 
Nothing technically prevent it from running using make and on linux, but this build was made using
Visual Studio 2013. Therefor the binaries included require VS2013 redistributable (included).

The Satelite model in the sattelite version of the software has been (painfully) extracted 
from Civilization Beyond Earth by Firaxis (C) 2013.

Right now, this is a super barebone demo involving :
Per Pixel Phong Shading, Mesh Loading, Texture Mapping, Rotating screen, Anti-Aliasing,
and Shader Rendering (the main reason it is so barebone: it took a day to remove all the bugs in the
mesh loading and shader code, I got 5 white triangle rotating by noon, at 5pm I got phong shading
with incorrect vertices, and I finally made texture mapping work at 7pm).

There is no buttons, no extra feature, nothing remarkable.

To run the executables, go to SatDefense\SatDefense and run SatDefense-earth.exe or SatDefense-sattelite.exe
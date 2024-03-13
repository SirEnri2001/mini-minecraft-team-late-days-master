Xinghua Han:
Milestone 1:
I implemented the generation of mountains and plains using a wraped Perlin Noise and Worley Noise. I found a bug in Worley Noise implementation, that we should check the scatter points of perlin noise in 5x5 range instead of 3x3 range. 3x3 range will give out a strange output that clamped in each cell's border. The parameters can be adjusted further when we finish our project. I will try to use GPU computing instead of CPU to generate terrain block types.

Milestone 2: 
I implemented the post effect with framebuffer and texture on a quad on a screen space. I use a postprocess shader to make changes in fragment. When using framebuffer, we should perform draw call twice. The first time  we render the whole scene onto a framebuffer that we defined. The second draw call render the texture of buffered screen onto a fullscreen quadrilateral. And we may perform posteffect shader to modify that. In the cave system generation, I use a 3D perlin noise as a main noise, and using another 2D Perlin noise to alter the height of each block of the cave. So the result cave came to be more realistic and natural. 

Milestone 3：
Procedural Sky Based on 3D Perlin and Worley Noise: I designed a beautiful and realistic sky using 3D Perlin and Worley noise algorithms, which dynamically alters in response to in-game time and weather conditions.

Particle System for Weather Simulation: Employing the OpenGL Geometry Shader, I crafted an advanced particle system to simulate varying weather conditions, such as rain and snow. This system brings an additional layer of dynamism and interaction to the game.
========================================
Yang Yu
Milestone1:
I implemented game engine tick function and player physics. The most challenege parts I met are gound collision check and the functions that player used to remove and put block. For the ground collision check part, I used getblockat() function to check if player hit the block and find the shortest distance. Then, I changed my movevector variable and velocity based on the shortest distance I found. For removing and putting blocks part, I find the point that the ray hit on the block and minus block's center point. Then, use x,y,z difference to determine the face I need to put my blocks next to. 

Milestone 2:
During the implementation of this week's OpenGL features, I focused on several key tasks. Loading textures into OpenGL involved adapting existing code to read image files and pass them to the GPU, and incorporating Qt's resource system for effective texture file handling. While modifying the shader to include sampler2D for texture application and introducing time-based UV animation, a notable challenge was how to identify the blocks in the shader, like lava or water, so that I can only animate Lava and water blocks. To achieve this implementation, I took uv coordinates from the texture and use that uv coordinates to identify the water and lava blocks.

Milestone 3：
Fluid Simulation: I implemented a fluid simulation system that realistically mimics the movement and behavior of liquids, providing dynamic and interactive experiences with water bodies and other fluids.

Water Wave Effects: I developed sophisticated water wave patterns that respond realistically to environmental factors, adding depth and authenticity to the water surfaces in our game.

Fog Effects: I introduced a distance-based fog system that adds atmospheric depth to the environment, crucial for creating lifelike and immersive outdoor scenes.

Post-process Camera Overlay: Additionally, I created a post-processing effect for the camera to simulate the visual distortion and color changes when the player is submerged in different types of fluids.

Frame Buffer Abstraction: I created a frame buffer class to help with switch between frame buffers and binding textures to different slots.
========================================
Zhanbo Lin:
Milestone1:
I implemented efficient rendering and terrain chunks generation. I write checkAndAddChunks function to progressively generate chunks based on player's X & Z coordinate. I wrote createVBOdata function to check every block within a chunk and its neighbors, then render it if it's an outside block. I used a GL buffer called m_bufAggregate to store the position, normal and color data, and accessed them through different layouts. I have a segment fault when getting GL errors. It turned out to be I stored the aggregated data using GL_ELEMENT_ARRAY_BUFFER instead of GL_ARRAY_BUFFER in my function called bufferVBOData. 

Milestone2:
I implemented multithreading functionality to optimize terrain expansion, preventing gameplay slowdown. The primary challenge involved managing shared memory using mutexes, and opengl context error occurred when trying to call glGenBuffers(), leading to crashes. I resolved this with the help from Xinghua, he helped me to locate the issue. Additionally, I ensured proper communication between the main thread and worker threads, allowing seamless data exchange. I also helped Xinghua Han to adjust player's movement, so the game experience can be more smooth.Overall, the integration of multithreading substantially improved the efficiency of dynamic terrain generation, providing players with a seamless and responsive gaming experience. 

Milestone3：
Shadow Mapping: I implemented realtime shadow using shadow mapping, enhancing the lighting realism. This feature allows for dynamic shadows that adapt to the sun. 

Distance Fog: Furthermore, I developed an effective distance fog effect, adding a layer of depth and intrigue to our environments, especially in creating atmospheric and immersive outdoor scenes.

Mini Map: I implemented a mini map shown on the upper left corner of the screen.
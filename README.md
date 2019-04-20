# Minecraft
CS 378H: Honors Graphics

## Extra Credit:

### Shadow Mapping (20 pts)
We shadow map with an orthographic projection to simulate directional light.
The light oscillates in the sky.

### Dynamic Modifications to Terrain (10 pts)
In creative mode, accessible by pressing "F" without "Ctrl", you have some nicer controls
(Shift to sink, Space to fly), and you can destroy blocks. This mode also does not clip against the world.

These changes are persistent as we save the chunks in memory. The terrain is theoretically infinite, but we do not load/unload to disk so the memory gets eaten up over time.

### Pick up and place different blocks (20 pts)
Again, in Creative mode, you can destroy blocks by Left Clicking.
You can Middle Click to select a block type and then Right Click to place it.
Because we're emulating creative mode, there are no limits to how many blocks you can place.

### Air-solid face optimization (10 pts?)
We only render faces of solid cubes that face air. This makes it so that we draw a very
small amount of faces.
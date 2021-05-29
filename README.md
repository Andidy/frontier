# frontier
A game engine for Retro-style strategy games.
![Preview of Game](./screenshots/preview.png)

Tile Texturing using Wang 2-edge 2-corner tiles a.k.a. Wang Blob Tiles.

![Preview of Wang Blob Tiles](./screenshots/wang_blob_example.png)

And here I've added a way to have multiple variants of the subtiles which construct the Wang Blob Tile graphics.
You can see some of the subtile variants along the top water edge.

![Wang Blob Tiles With Variants](./screenshots/wang_blob_with_subtile_variants.png)

An example of a town with various buildings (textured and untextured), the yellow hatching shows a building which hasn't finished construction.

![Initial Buildings and Resource Generation](./screenshots/buildings.png)

I've added basic unit pathing and the units will walk the path one tile per tick.

![Initial basic unit pathing](./screenshots/basic_pathing.png)

## Features:
 - Software Rendered
 - Camera Movment and Zoom levels.
 - Viewport Clipping (game map elements outside of the game window are not drawn)
 - Transparency (by discarding pixels with 0 in alpha channel)
 - UI Elements : Rectangles, Images, ASCII Text (w/ text and background coloring), Buttons, Tilemaps, Game Window
 - Game Map Editing
 - Unit Movement, Basic Pathing, Attacking, State Menu
 - Program & Game Settings loaded from Json
 - Cycle Counters for performance analysis
 - Production buildings refine resources into other resources
 - Building templates use Json for simple building modification
 
### Feature Showcases:
- Software Rendering & Camera Movement: https://youtu.be/LpRilZxemBQ
- Initial UI & Unit Movement: https://youtu.be/of5REwYvzBI
- Unit Combat: https://youtu.be/89JbdbfRlow
- Tilemap Editing: https://youtu.be/sTUpxr7p5C8
- Tilemap Editing with Different Tilesets: https://youtu.be/KST4Q0XWDps
- Wang Blob Tiles for pretty transitions between tile types: https://youtu.be/zu53zhWPZ2Y

## Goals (in no particular order):
- Improvements to UI Text Layout
- Game Map Saving & Loading
- Unit Saving & Loading
- Unit Placement
- Program & Game Settings Saving
- UI Anchors to improve Layout

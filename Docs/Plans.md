The following is the overall design goals for the feel of the game, what the general gameplay goals are and what features to implement.
This is what the game would become with enough time and effort. These are vary rough ideas and are subject to change as they get implemented

### GamePlay:
#### Environment:
* Focused on more exploration then building
* Most building and destruction to be done in large chunks, due to the nature and fidelity of the surface net system
* Player building can be done using an entity system as opposed to the voxel system, allowing for 2 seperate planes, one with more fidelity
   (This is to alleviate some of the problems of fidelity when attempting to build using a surface netted voxel)
* All destruction however will be done using the voxel system (due to how well the system reacts to destruction as opposed to construction)
* Rough construction of voxels can be done using items and the like (Such as a magic wand to build a rough dirt bridge to cross an area, allow for decaying voxels)
* Make use of the realistic physics as much as possible for all the entities and items
* The key gameplay goal is EXPLORATION, give the player access to better and better items and tougher and tougher mobs throughout player progression
* Take advantage of the ability to go generate chunks vertically and horizontally for gameplay progression
* Different biomes depending on world height (Overworld, underground, deeper underground, sky, all for different points in player progression with different mob difficulty)
#### Items:
* Found randomly throughout the world, and are a key to player progression (Maybe some crafted to minimize player frustration due to bad rng)
* Have random locations throughout the world which introduce powerful items to the player, some easy to get to (ala overworld abandoned house) with
* generic rewards, to very difficult to reach (ala deep underground) with much more powerful rewards.
* Let the item system be very customizable, allow for easy implementation, and perhaps even player creation (such as modding tools)
#### Enemies
* Varied and nuanced
* Difficulty scaling depending on where they are found (Enemies deep underground much more difficult then generic enemies)
#### Movement
* Movement should feel fluid, and have a sense of progression.
* Go from normal movement, to unlocked double jumps, to eventually flight, as the player progresses through the area
#### Building
* Building and destruction will be less done by the player and more done by the players items (Use explosives for destruction, and magic items for construction)
* Real player building can be premade using assets (such as a starting shack)
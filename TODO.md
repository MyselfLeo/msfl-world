Resources:

- Create world.copy(other), which returns a new resource with
  the same values as other (but is a new instance)

Box:

- Make Box::inside work with all boxes (not only bounding boxes)

Resources & Components:

- Material : add all parameters relative to rendering (see Material.hpp)
- Figure out how to make "reload" work for different resources (or dump this idea ?)
  > If so, maybe create ModelTool::load_from_file ?
- Add a way to delete unused resources.

- Components: have default values for -almost- everything

- Components should "depend on" other components and fail to initialize if not present.

Camera:

- switch between projection & orthographic mode

Model:

Remove Meshgroup ! a Model should only have a list of meshes. (we're back at the start lol)

World:

- Add detach_component

Orbiter (and possibly others):

- Find a way to automatically update this kind of components. May do OrbiterSystem instead of orbiter.update ?

Bugs :

- when the camera position is extreme, the cubemap looks weird (precision errors ?)

Transform :

- Create HierarchicTransform, like transform (in fact it extends Transform) but with a reference
  to a transform. Then to get from model to world we apply the first transform then the 2nd.
  We can build a transform tree with that.

Materials :
- Maybe add StaticMaterial & DynamicMaterial. Materials are updated each frame on the GPU but for most
  of them it may not be necessary (thus we'd use StaticMaterial).
Resources & Components:

- Material : add all parameters relative to rendering (see Material.hpp)
- Figure out how to make "reload" work for different resources (or dump this idea ?)
  > If so, maybe create ModelTool::load_from_file ?
- Add a way to delete unused resources.

- Components: have default values for -almost- everything

Camera:

- switch between projection & orthographic mode

Model:

- Alternative: Keep Mesh, do MeshGroup instead of Model
- Delete Mesh as a resource, clean up wrld::Model (split from file and from raw data)
  > This will make Model::aggregate obsolete and require something like reload() or build()
- When importing Model from file, allow either : merge all meshes into the model (1 material), or create one model
  for each mesh
- The material will be attached to the model
- This will also reduce drastically memory usage as everything is copied twice or even thrice currently
- Create a "ModelGroup" resource to be attached to an entity
  > Allows an entity to have multiple models attached to it.
  > As 1 model = 1 material, this allows to have different materials on the entity model.

```
Mesh: Vertices + Elements,      stored in a VAO

ModelImporter: tool helping Model importation.

Model: A model should store X meshes, with X being its number of materials.
A model has one transform. Because we have to do 1 draw call for each material / transform pair, we have nbTransforms * NbMaterials draw calls.


ModelGroup: A group of models. All models have the same transform. It allows an Entity to have multiple models.


-> Problems: we want to reduce memory usage and draw call count.

With (ex) 100 models of 3 materials in ModelGroup, we have 100 * 3 draw_calls. However, all models have the same transform, so we should really only do 3 draw calls.

-> Solution: ModelGroup should NOT be a collection of models. It should instead be a regular model with the given additions :
             1. Keep the VBO
             2. Sort the EBO by groups
             3. Store the bounding box of each submodel
             4. Do frustum culling on each submodel and, why not, on other levels (group submodels by 2 for example).
```

World:

- Add detach_component

Bugs :

- when the camera position is extreme, the cubemap looks weird (precision errors ?)
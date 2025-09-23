Resources & Components:

- Material : add all parameters relative to rendering (see Material.hpp)
- Figure out how to make "reload" work for different resources (or dump this idea ?)
  > If so, maybe create ModelTool::load_from_file ?

Camera:

- switch between projection & orthographic mode

Model:

- Delete Mesh as a resource, clean up wrld::Model (split from file and from raw data)
  > This will make Model::aggregate obsolete and require something like reload() or build()
- When importing Model from file, allow either : merge all meshes into the model, or create one model
  for each mesh

Bugs :

- when the camera position is extreme, the cubemap looks weird (precision errors ?)
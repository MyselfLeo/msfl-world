Box & all:
- Change FLT_MIN to -FLT_MAX

Resources & Components:

- Material : add all parameters relative to rendering (see Material.hpp)
- Figure out how to make "reload" work for different resources (or dump this idea ?)
  > If so, maybe create ModelTool::load_from_file ?
- Add a way to delete unused resources.

- Components: have default values for -almost- everything

Camera:

- switch between projection & orthographic mode

Model:

Par contre, peut-être qu'on peut se débarrasser de suffisamment de données membres (model_directory, model_path...) pour
ne pas avoir besoin de faire de sous-classes.

On pourra alors garder l'architecture Model::from_file, Model::from_mesh, Model::from_meshgroup, Model::from_box...

Stockage des mesh dans Model:

std::vector<Rc<Material>> materials;
std::vector<MeshGroup> groups;

std::vector<int> group_material; // Associe à chaque group un material.

World:

- Add detach_component

Orbiter (and possibly others):
- Find a way to automatically update this kind of components. May do OrbiterSystem instead of orbiter.update ?

Bugs :

- when the camera position is extreme, the cubemap looks weird (precision errors ?)

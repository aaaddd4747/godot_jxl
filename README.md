An attempt at modifying [godot_qoi](https://github.com/DmitriySalnikov/godot_qoi) into an extension capable of loading and saving jpeg-xl images using [libjxl](https://github.com/libjxl/libjxl). 

### Requires:
You must have libjxl installed on your system.

### Design goal:
 - Use jxl's progressive loading feature to return instantly before the image is fully loaded and pray the image reference does not get invalidated while the rest of the image is loaded in another thread.

If this works, incredible effective loading speeds may be possible. If not, jxl is still pretty neat.

A notable wrench in the works: godot's internal image format does not support ICC profiles. I will simply be ignoring them if possible.
Please just use ordinary RGB for now.

Tasks:
---
- [x] rename everything
- [ ] make it load jxl
- [ ] make it save jxl

Current state:
---
It builds but godot wont load it for some reason. It's like it isnt even there!
In theory it currently loads jxl images, but I cannot test it like this.
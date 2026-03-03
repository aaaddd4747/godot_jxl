an attempt at modifying [godot_qoi](https://github.com/DmitriySalnikov/godot_qoi) into an extension capable of loading and saving jpeg-xl images using [libjxl](https://github.com/libjxl/libjxl). 

design goal:
use jxl's progressive loading feature to return instantly before the image is fully loaded and pray the image reference does not get invalidated.
if this works, incredible effective loading speeds may be possible. if not, jxl is still pretty neat.

# TODOs list:
## Features:
* adjust bvh interface (done)
* add bvh traverse + trivial build (done)
* add bvh first non trivial build
* optimize bvh well
## Refactoring:
* return single intersection point after switching to GLTF
* aggregate object distribution into object or initialize by default
  (in that case object should implement ObjectDistribution interface)
* better distribution creating without case with dynamic casts

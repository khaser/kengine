# TODOs list:

## Performance:
* add early out heuristic
* SAH bvh build
* reorder Node values to increase cache-hit

## Refactoring:
* return single intersection point after switching to GLTF
* aggregate object distribution into object or initialize by default
  (in that case object should implement ObjectDistribution interface)
* better distribution creating without case with dynamic casts

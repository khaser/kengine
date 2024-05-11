#pragma once
#include "Primitives.h"
#include "Object.h"

#include <vector>
#include <optional>

struct Node {
    ssize_t start;
    ssize_t len;
    ssize_t left;
    ssize_t right;
    Box aabb;
};

using resT = std::optional<std::pair<Object, Intersection>>;

struct BVH {
public:
    BVH() {};
    BVH(std::vector<Object> &&objs_arg);

    resT get_intersect(const Ray& ray, bool early_out) const;

private:
    resT get_intersect_(ssize_t node_idx, const Ray& ray, bool early_out) const;
    ssize_t build_bvh(std::vector<Object>::iterator begin, std::vector<Object>::iterator end);

private:
    std::vector<Object> objs;
    std::vector<Object>::iterator bvh_end;
    std::vector<Node> tree;
    int root_node;
    static const size_t term_size = 16;
};

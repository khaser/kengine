#pragma once
#include "Primitives.h"
#include "Object.h"

#include <bits/ranges_algo.h>
#include <bits/ranges_base.h>
#include <memory>
#include <vector>
#include <algorithm>
#include <optional>
#include <ranges>

struct Node {
    int start;
    int len;
    int left;
    int right;
    Box aabb;
};

using resT = std::optional<std::pair<Object, Intersection>>;
using namespace std::views;
namespace ranges = std::ranges;

struct BVH {
public:
    BVH() {};
    BVH(std::vector<Object> &&objs_arg) : objs(objs_arg) {
        bvh_end = std::partition(objs.begin(), objs.end(), [] (const Object& obj) -> bool {
            return dynamic_pointer_cast<Plane>(obj.geometry) == 0;
        });

        build_bvh(0, bvh_end - objs.begin());
        root_node = tree.size() - 1;
    }

    resT get_intersect(const Ray& ray) const {
        auto inters = found_inters(ranges::subrange(bvh_end, objs.end()), ray);
        auto bvh_inter = get_intersect_(root_node, ray);
        if (bvh_inter.has_value()) {
            inters.push_back(bvh_inter.value());
        }
        return *ranges::min_element(inters, res_cmp);
    }

private:
    static bool res_cmp(const resT &a, const resT &b) {
        if (!a.has_value()) return false;
        if (!b.has_value()) return true;
        return a.value().second.t < b.value().second.t;
    };

    static std::vector<resT> found_inters(ranges::forward_range auto objs, const Ray& ray) {
        auto node_inters_rng = objs
            | transform([&ray] (const Object& obj) -> resT {
                auto obj_inters = obj.geometry->get_intersect(ray);
                auto it = ranges::min_element(obj_inters, {}, &Intersection::t);
                if (it == obj_inters.end()) {
                    return std::nullopt;
                } else {
                    return std::make_pair(obj, *it);
                }
            });
        return {node_inters_rng.begin(), node_inters_rng.end()};
    }

    void build_bvh(int start, int end) {
        Box box({200000, 200000, 200000});
        tree.emplace_back(Node {start, end - start, -1, -1, box});
    }

    resT get_intersect_(int node_idx, const Ray& ray) const {

        const Node &node = tree[node_idx];

        if (node.aabb.get_intersect(ray).empty()) {
            return std::nullopt;
        }

        auto inters = found_inters(ranges::subrange(objs.begin(), bvh_end) | drop(node.start) | take(node.len), ray);

        for (const int &child : {node.left, node.right}) {
            if (child != -1) {
                auto inter = get_intersect_(child, ray);
                if (inter.has_value()) {
                    inters.emplace_back(inter.value());
                }
            }
        }

        return *std::min_element(inters.begin(), inters.end(), res_cmp);
    }

private:
    std::vector<Object> objs;
    std::vector<Object>::const_iterator bvh_end;
    std::vector<Node> tree;
    int root_node;
};

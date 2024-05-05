#pragma once
#include "Primitives.h"
#include "Object.h"

#include <bits/ranges_algo.h>
#include <bits/ranges_base.h>
#include <memory>
#include <vector>
#include <algorithm>
#include <numeric>
#include <optional>
#include <ranges>

struct Node {
    ssize_t start;
    ssize_t len;
    ssize_t left;
    ssize_t right;
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

        root_node = build_bvh(objs.begin(), bvh_end);
    }

    resT get_intersect(const Ray& ray, bool early_out) const {
        auto oth_inter = found_inters(ranges::subrange(static_cast<std::vector<Object>::const_iterator>(bvh_end), objs.end()), ray);
        return better(oth_inter, get_intersect_(root_node, ray, early_out));
    }

private:
    resT get_intersect_(ssize_t node_idx, const Ray& ray, bool early_out) const {
        const Node &node = tree[node_idx];

        if (node.aabb.get_intersect(ray).empty()) {
            return std::nullopt;
        }

        auto res = found_inters(ranges::subrange(objs.begin(), bvh_end) | drop(node.start) | take(node.len), ray);

        for (const ssize_t &child : {node.left, node.right}) {
            auto aabb_inter = node.aabb.get_intersect(ray);
            if (child != -1 &&
                    (!early_out ||
                     !res.has_value() ||
                     ranges::min_element(aabb_inter, {}, &Intersection::t)->t < res->second.t)) {
                res = better(res, get_intersect_(child, ray, early_out));
            }
        }

        return res;
    }

    static resT better(const resT &a, const resT &b) {
        if (res_cmp(a, b)) {
            return a;
        } else {
            return b;
        }
    }

    static resT found_inters(ranges::forward_range auto objs, const Ray& ray) {
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
        /* std::vector<resT> inters(node_inters_rng.begin(), node_inters_rng.end()); */
        /* return *std::min_element(inters.begin(), inters.end(), res_cmp); */
        return *ranges::min_element(node_inters_rng, res_cmp);
    }

    static bool res_cmp(const resT &a, const resT &b) {
        if (!a.has_value()) return false;
        if (!b.has_value()) return true;
        return a->second.t < b->second.t;
    };

    template <class RandomIt>
    ssize_t build_bvh(RandomIt begin, RandomIt end) {
        Box node_aabb = std::transform_reduce(begin + 1, end,
            begin->geometry->AABB(),
            [] (const Box& a, const Box& b) {
                return a | b;
            },
            [] (const Object& obj) { return obj.geometry->AABB(); }
        );
        if (end - begin <= term_size) {
            node_aabb.bump();
            /* std::cerr << "Create term node with " << end - begin << " objects\n"; */
            tree.emplace_back(begin - objs.begin(), end - begin, -1, -1, node_aabb);
            return tree.size() - 1;
        }

        auto pivot = end;
        node_aabb.bump();
        while (true) {
            pivot = std::partition(begin, end, [&node_aabb] (const Object &obj) {
                auto sel = node_aabb.size.maj();
                return obj.geometry->mid() % sel > node_aabb.position % sel;
            });
            if (pivot == begin) {
                node_aabb = Box(node_aabb.Min(), node_aabb.Max() - node_aabb.size.maj() * node_aabb.size);
            } else if (pivot == end) {
                node_aabb = Box(node_aabb.Min() + node_aabb.size.maj() * node_aabb.size, node_aabb.Max());
            } else {
                break;
            }
        }

        /* std::cerr << "Create split node: " << pivot - begin << " in left child, and " << end - pivot << " in right child\n"; */
        ssize_t L = build_bvh(begin, pivot);
        ssize_t R = build_bvh(pivot, end);
        tree.emplace_back(begin - objs.begin(), 0, L, R, tree[L].aabb | tree[R].aabb);
        return tree.size() - 1;
    }

private:
    std::vector<Object> objs;
    std::vector<Object>::iterator bvh_end;
    std::vector<Node> tree;
    int root_node;
    static const size_t term_size = 16;
};

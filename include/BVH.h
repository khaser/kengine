#pragma once

#include "Primitives/Ray.h"
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

namespace RawBVH {

template<class T, class F, class Map, class Merge, class Geom>
struct BVH {

using objsIt = std::vector<T>::const_iterator;

public:
    BVH() {};
    BVH(F ini, objsIt begin, objsIt end) : ini(ini), objs(begin, end) {
        root_node = build_bvh(objs.begin(), objs.end());
    }

    F get_intersect(const Ray& ray, bool early_out) const {
        return get_intersect_(root_node, ray, early_out);
    }

private:
    F get_intersect_(ssize_t node_idx, const Ray& ray, bool early_out) const {
        if (node_idx == -1) return ini;
        const Node &node = tree[node_idx];

        if (node.aabb.get_intersect(ray).empty()) {
            return ini;
        }

        std::vector<F> node_inters;
        std::transform(objs.begin() + node.start, objs.begin() + node.start + node.len, std::back_inserter(node_inters), Map(ray));
        F res = std::accumulate(node_inters.begin(), node_inters.end(), ini, Merge());

        for (const auto &child : {node.left, node.right}) {
            res = Merge()(res, get_intersect_(child, ray, early_out));
        }
        return res;

        // TODO: restore selection better child traversal
        /* { */
        /*     auto first_inter_by_idx = [&] (ssize_t idx) { */
        /*         return best_inter(idx == -1 ? std::vector<Intersection>() : tree[idx].aabb.get_intersect(ray)); */
        /*     }; */

        /*     auto left_first = first_inter_by_idx(node.left); */
        /*     auto right_first = first_inter_by_idx(node.right); */
        /*     if (inter_cmp(left_first, right_first)) { */
        /*         childs = {std::make_pair(node.left, left_first), std::make_pair(node.right, right_first)}; */
        /*     } else { */
        /*         childs = {std::make_pair(node.right, right_first), std::make_pair(node.left, left_first)}; */
        /*     } */
        /* } */

        /* for (const auto &[child, aabb_inter] : childs) { */
        /*     if (child == -1) continue; */
        /*     if (!early_out || !res.has_value() || inter_cmp(aabb_inter, res->second)) { */
        /*         res = Merge(res, get_intersect_(child, ray, early_out)); */
        /*     } */
        /* } */

        /* return res; */
    }

    ssize_t build_bvh(std::vector<T>::iterator begin, std::vector<T>::iterator end) {
        if (begin == end) return -1;
        Box node_aabb = std::transform_reduce(begin + 1, end,
            (Geom() (*begin))->AABB(),
            [] (const Box& a, const Box& b) {
                return a | b;
            },
            [] (const T& obj) { return (Geom() (obj))->AABB(); }
        );
        if (end - begin <= term_size) {
            node_aabb.bump();
            /* std::cerr << "Create term node with " << end - begin << " objects\n"; */
            tree.push_back({begin - objs.begin(), end - begin, -1, -1, node_aabb});
            return tree.size() - 1;
        }

        auto pivot = end;
        while (true) {
            pivot = std::partition(begin, end, [&node_aabb] (const T &obj) {
                auto sel = node_aabb.size.maj();
                return (Geom() (obj))->mid() % sel > node_aabb.position % sel;
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
        tree.push_back({begin - objs.begin(), 0, L, R, tree[L].aabb | tree[R].aabb});
        return tree.size() - 1;
    }

private:
    std::vector<T> objs;
    std::vector<Node> tree;
    int root_node; // may be -1 if bvh is empty
    F ini;
    static const size_t term_size = 16;
};

}

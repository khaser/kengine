#pragma once

#include "Primitives/Ray.h"
#include "Object.h"

#include <vector>
#include <optional>

struct Node {
    ssize_t start;
    ssize_t len;
    Node *left;
    Node *right;
    Box aabb;
};

namespace RawBVH {

template<class T, class F, class Map, class Merge, class Geom, class EarlyOut, class Traverse>
struct BVH {

using objsIt = std::vector<T>::const_iterator;

public:
    BVH() {};
    BVH(F ini, objsIt begin, objsIt end) : ini(ini), objs(begin, end) {
        tree.reserve(objs.end() - objs.begin()); // prevent reallocations to make dirty hacks in build work
        root_node = build_bvh(objs.begin(), objs.end());
    }

    F get_intersect(const Ray& ray, bool early_out) const {
        if (root_node == NULL) return ini;
        return get_intersect_(root_node, ray, early_out);
    }

private:
    F get_intersect_(const Node *node, const Ray& ray, bool early_out) const {
        if (node->aabb.get_intersect(ray).empty()) {
            return ini;
        }

        std::vector<F> node_inters;
        std::transform(objs.begin() + node->start, objs.begin() + node->start + node->len, std::back_inserter(node_inters), Map(ray));
        F res = std::accumulate(node_inters.begin(), node_inters.end(), ini, Merge());

        for (Node* child : Traverse() (ray, node)) {
            if (child != NULL && !(EarlyOut() (ray, res, child))) {
                res = Merge()(res, get_intersect_(child, ray, early_out));
            }
        }
        return res;

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
    }

    Node* build_bvh(std::vector<T>::iterator begin, std::vector<T>::iterator end) {
        if (begin == end) return NULL;
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
            tree.push_back({begin - objs.begin(), end - begin, NULL, NULL, node_aabb});
            return &tree.back();
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
        Node *L = build_bvh(begin, pivot);
        Node *R = build_bvh(pivot, end);
        tree.push_back({begin - objs.begin(), 0, L, R, L->aabb | R->aabb});
        return &tree.back();
    }

private:
    std::vector<T> objs;
    std::vector<Node> tree;
    Node *root_node;
    F ini;
    static const size_t term_size = 8;
};

}

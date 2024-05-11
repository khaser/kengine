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

template<class T, class F, class Map, class Merge>
struct BVH {

using objsIt = std::vector<T>::const_iterator;

public:
    BVH() {};
    BVH(F ini, std::vector<T> &&objs_arg) : ini(ini), objs(objs_arg) {
        // TODO: move planes processing into Scene, bvh shouldn't care about it
        bvh_end = std::partition(objs.begin(), objs.end(), [] (const T& obj) -> bool {
            return dynamic_pointer_cast<Plane>(obj.geometry) == 0;
        });

        root_node = build_bvh(objs.begin(), bvh_end);
    }

    F get_intersect(const Ray& ray, bool early_out) const {
        F oth_inter = found_inters(bvh_end, objs.end(), ray);
        return Merge() (oth_inter, get_intersect_(root_node, ray, early_out));
    }

private:
    F get_intersect_(ssize_t node_idx, const Ray& ray, bool early_out) const {
        const Node &node = tree[node_idx];

        if (node.aabb.get_intersect(ray).empty()) {
            return std::nullopt;
        }

        F res = found_inters(objs.begin() + node.start, objs.begin() + node.start + node.len, ray);

        for (const auto &child : {node.left, node.right}) {
            if (child == -1) continue;
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

    F found_inters(objsIt begin, objsIt end, const Ray& ray) const {
        std::vector<F> node_inters;
        std::transform(begin, end, std::back_inserter(node_inters), Map(ray));
        return std::accumulate(node_inters.begin(), node_inters.end(), ini, Merge());
    }

    ssize_t build_bvh(std::vector<T>::iterator begin, std::vector<T>::iterator end) {
        Box node_aabb = std::transform_reduce(begin + 1, end,
            begin->geometry->AABB(),
            [] (const Box& a, const Box& b) {
                return a | b;
            },
            [] (const T& obj) { return obj.geometry->AABB(); }
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
        tree.push_back({begin - objs.begin(), 0, L, R, tree[L].aabb | tree[R].aabb});
        return tree.size() - 1;
    }

private:
    std::vector<T> objs;
    std::vector<T>::iterator bvh_end;
    std::vector<Node> tree;
    int root_node;
    F ini;
    static const size_t term_size = 16;
};

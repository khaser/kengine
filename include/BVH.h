#pragma once

#include "Primitives/Ray.h"
#include "Primitives/AABB.h"
#include "Object.h"

#include <vector>
#include <optional>
#include <cassert>

struct Node {
    ssize_t start;
    ssize_t len;
    ssize_t left;
    ssize_t right;
    AABB aabb;
};

namespace RawBVH {

std::optional<Intersection> best_inter(std::shared_ptr<Geometry> geom, const Ray &r);

template<class T, class F, class Map, class Merge, class Geom, class EarlyOut>
struct BVH {

using objsIt = std::vector<T>::const_iterator;

public:
    BVH() {};
    BVH(F ini, objsIt begin, objsIt end) : ini(ini), objs(begin, end) {
        tree.reserve(objs.end() - objs.begin()); // prevent reallocations to make dirty hacks in build work
        root_node = build_bvh(objs.begin(), objs.end());
        std::cerr << "BVH statistics:\n";
        std::cerr << "Total nodes: " << tree.size() << '\n';
        auto it = std::max_element(tree.begin(), tree.end(),
                    [&] (const Node &a, const Node& b) {
                        return a.len < b.len;
                  });
        if (it != tree.end()) {
            std::cerr << "Largest node size: " << it->len << '\n';
        } else {
            std::cerr << "Empty tree!!\n";
        }
    }

    F get_intersect(const Ray& ray, bool early_out) const {
        return get_intersect_(root_node, ray, early_out);
    }

private:
    F get_intersect_(ssize_t node_idx, const Ray& ray, bool early_out) const {
        if (node_idx == -1) return ini;
        const Node &node = tree[node_idx];
        std::vector<F> node_inters;
        std::transform(objs.begin() + node.start, objs.begin() + node.start + node.len, std::back_inserter(node_inters), Map(ray));
        F res = std::accumulate(node_inters.begin(), node_inters.end(), ini, Merge());

        std::vector<std::pair<ssize_t, float>> childs;
        auto helper = [&ray, &res, &childs, this] (ssize_t node_idx) {
            if (node_idx == -1) return;
            auto inter_t = tree[node_idx].aabb.get_intersect(ray);
            if (inter_t >= 1e20) return;
            childs.emplace_back(node_idx, inter_t);
        };
        helper(node.left);
        helper(node.right);
        std::sort(childs.begin(), childs.end(), [] (auto &a, auto &b) { return a.second < b.second; });

        for (auto &[child, inter_t] : childs) {
            if (child != -1 && !(EarlyOut() (res, inter_t))) {
                res = Merge() (res, get_intersect_(child, ray, early_out));
            }
        }
        return res;
    }

    ssize_t build_bvh(std::vector<T>::iterator begin, std::vector<T>::iterator end) {
        if (begin == end) return -1;
        AABB node_aabb = std::transform_reduce(begin + 1, end,
            (Geom() (*begin))->get_aabb(),
            [] (const AABB& a, const AABB& b) {
                return a | b;
            },
            [] (const T& obj) { return (Geom() (obj))->get_aabb(); }
        );
        if (end - begin <= term_size) {
            /* std::cerr << "Create term node with " << end - begin << " objects\n"; */
            tree.push_back({begin - objs.begin(), end - begin, -1, -1, node_aabb});
            return tree.size() - 1;
        }

        auto pivot = end;
        Vec3<float> sel = node_aabb.size().maj();
        pivot = std::partition(begin, end, [&sel, &node_aabb] (const T &obj) {
            return (Geom() (obj))->get_aabb().position() % sel > node_aabb.position() % sel;
        });
        if (pivot == begin || pivot == end) {
            // Case when exists one object, which AABB contains other objs
            // Put huge object into left node, and continue build BVH for others in right node
            pivot = std::partition(begin, end, [&sel, &node_aabb] (const T &obj) {
                return fabs(((Geom() (obj))->get_aabb().position() - node_aabb.position()) % sel) < 1e-5;
            });

            if (pivot == begin || pivot == end) {
                tree.push_back({begin - objs.begin(), end - begin, -1, -1, node_aabb});
                return tree.size() - 1;
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
    ssize_t root_node;
    F ini;
    static const size_t term_size = 1;
};

}

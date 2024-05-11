#include "BVH.h"

#include <algorithm>
#include <numeric>

using objsIt = std::vector<Object>::const_iterator;

namespace {

bool res_cmp(const resT &a, const resT &b) {
    if (!a.has_value()) return false;
    if (!b.has_value()) return true;
    return a->second.t < b->second.t;
};

resT better(const resT &a, const resT &b) {
    if (res_cmp(a, b)) {
        return a;
    } else {
        return b;
    }
}

bool inter_cmp(const std::optional<Intersection> &a, const std::optional<Intersection> &b) {
    if (!a.has_value()) return false;
    if (!b.has_value()) return true;
    return a->t < b->t;
};

std::optional<Intersection> best_inter(const std::vector<Intersection>& inters) {
    auto it = std::min_element(inters.begin(), inters.end(), inter_cmp);
    if (it == inters.end()) {
        return std::nullopt;
    } else {
        return *it;
    }
};

resT found_inters(objsIt begin, objsIt end, const Ray& ray) {
    std::vector<resT> node_inters;
    std::transform(begin, end, std::back_inserter(node_inters), [&ray] (const Object& obj) -> resT {
        auto inter = best_inter(obj.geometry->get_intersect(ray));
        if (!inter) {
            return std::nullopt;
        } else {
            return std::make_pair(obj, inter.value());
        }
    });
    auto it = std::min_element(node_inters.begin(), node_inters.end(), res_cmp);
    if (it == node_inters.end()) {
        return std::nullopt;
    } else {
        return *it;
    }
}

} // anonymous namespace

BVH::BVH(std::vector<Object> &&objs_arg) : objs(objs_arg) {
    bvh_end = std::partition(objs.begin(), objs.end(), [] (const Object& obj) -> bool {
        return dynamic_pointer_cast<Plane>(obj.geometry) == 0;
    });

    root_node = build_bvh(objs.begin(), bvh_end);
}

resT BVH::get_intersect(const Ray& ray, bool early_out) const {
    auto oth_inter = found_inters(bvh_end, objs.end(), ray);
    return better(oth_inter, get_intersect_(root_node, ray, early_out));
}

resT BVH::get_intersect_(ssize_t node_idx, const Ray& ray, bool early_out) const {
    const Node &node = tree[node_idx];

    if (node.aabb.get_intersect(ray).empty()) {
        return std::nullopt;
    }

    auto res = found_inters(objs.begin() + node.start, objs.begin() + node.start + node.len, ray);

    std::array<std::pair<ssize_t, std::optional<Intersection>>, 2> childs;
    // select better child traversal
    {
        auto first_inter_by_idx = [&] (ssize_t idx) {
            return best_inter(idx == -1 ? std::vector<Intersection>() : tree[idx].aabb.get_intersect(ray));
        };

        auto left_first = first_inter_by_idx(node.left);
        auto right_first = first_inter_by_idx(node.right);
        if (inter_cmp(left_first, right_first)) {
            childs = {std::make_pair(node.left, left_first), std::make_pair(node.right, right_first)};
        } else {
            childs = {std::make_pair(node.right, right_first), std::make_pair(node.left, left_first)};
        }
    }

    for (const auto &[child, aabb_inter] : childs) {
        if (child == -1) continue;
        if (!early_out || !res.has_value() || inter_cmp(aabb_inter, res->second)) {
            res = better(res, get_intersect_(child, ray, early_out));
        }
    }

    return res;
}

ssize_t BVH::build_bvh(std::vector<Object>::iterator begin, std::vector<Object>::iterator end) {
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

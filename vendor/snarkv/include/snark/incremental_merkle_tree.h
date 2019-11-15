#ifndef _INCREMENTAL_MERKLE_TREE_H_
#define _INCREMENTAL_MERKLE_TREE_H_

#include <array>
#include <deque>
#include <boost/optional.hpp>
//#include <boost/static_assert.hpp>
#include "uint256.h"
#include "zcash.h"
#include <fc/log/logger.hpp>
#include <snark/util.h>

namespace libzcash {

struct MerkleTreePack {
    std::string left;
    std::string right;
    std::vector<std::string> parents;
};

struct WitnessPack {
    MerkleTreePack tree;
    std::vector<std::string> filled;
    MerkleTreePack cursor;
    size_t cursor_depth = 0;
};

struct MerklePathPack {
    std::vector<std::vector<unsigned char>> path_bytes;
    uint64_t index_int;
};

class MerklePath {
public:
    std::vector<std::vector<bool>> authentication_path;
    std::vector<bool> index;

    MerklePath() { }

    MerklePath(std::vector<std::vector<bool>> authentication_path, std::vector<bool> index)
    : authentication_path(authentication_path), index(index) { }

    void toPack(MerklePathPack& pack) {
        assert(authentication_path.size() == index.size());
        pack.path_bytes.resize(authentication_path.size());
        for (size_t i = 0; i < authentication_path.size(); i++) {
            pack.path_bytes[i].resize((authentication_path[i].size()+7)/8);
            for (unsigned int p = 0; p < authentication_path[i].size(); p++) {
                pack.path_bytes[i][p / 8] |= authentication_path[i][p] << (7-(p % 8));
            }
        }
        pack.index_int = convertVectorToInt(index);
    }
};

template<size_t Depth, typename Hash>
class EmptyMerkleRoots {
public:
    EmptyMerkleRoots() {
        empty_roots.at(0) = Hash::uncommitted();
        for (size_t d = 1; d <= Depth; d++) {
            empty_roots.at(d) = Hash::combine(empty_roots.at(d-1), empty_roots.at(d-1), d-1);
        }
    }
    Hash empty_root(size_t depth) {
        return empty_roots.at(depth);
    }
    template <size_t D, typename H>
    friend bool operator==(const EmptyMerkleRoots<D, H>& a,
                           const EmptyMerkleRoots<D, H>& b);
private:
    std::array<Hash, Depth+1> empty_roots;
};

template<size_t Depth, typename Hash>
bool operator==(const EmptyMerkleRoots<Depth, Hash>& a,
                const EmptyMerkleRoots<Depth, Hash>& b) {
    return a.empty_roots == b.empty_roots;
}

template<size_t Depth, typename Hash>
class IncrementalWitness;

template<size_t Depth, typename Hash>
class IncrementalMerkleTree {

friend class IncrementalWitness<Depth, Hash>;

public:
    BOOST_STATIC_ASSERT(Depth >= 1);

    IncrementalMerkleTree() { }

    IncrementalMerkleTree(const MerkleTreePack& pack) {
        Hash h;
        if (!pack.left.empty()) {
            h.SetHex(pack.left);
            left = h;
        }

        if (!pack.right.empty()) {
            h.SetHex(pack.right);
            right = h;
        }

        parents.resize(pack.parents.size());
        for (size_t i = 0; i < parents.size(); i++) {
            if (!pack.parents[i].empty()) {
                h.SetHex(pack.parents[i]);
                parents[i] = h;
            }
        }
    }

    MerkleTreePack toMerkleTreePack() const {
        MerkleTreePack pack;
        if (left) {
            pack.left = (*left).GetHex();
        }
        if (right) {
            pack.right = (*right).GetHex();
        }
        for (auto& p : parents) {
            if (p) {
                pack.parents.emplace_back((*p).GetHex());
            } else {
                pack.parents.emplace_back("");
            }
        }

        return pack;
    }

    size_t DynamicMemoryUsage() const {
        return 32 + // left
               32 + // right
               parents.size() * 32; // parents
    }

    size_t size() const;

    void append(Hash obj);
    Hash root() const {
        return root(Depth, std::deque<Hash>());
    }
    Hash last() const;

    IncrementalWitness<Depth, Hash> witness() const {
        return IncrementalWitness<Depth, Hash>(*this);
    }

    static Hash empty_root() {
        return emptyroots.empty_root(Depth);
    }

    template <size_t D, typename H>
    friend bool operator==(const IncrementalMerkleTree<D, H>& a,
                           const IncrementalMerkleTree<D, H>& b);

private:
    static EmptyMerkleRoots<Depth, Hash> emptyroots;
    boost::optional<Hash> left;
    boost::optional<Hash> right;

    // Collapsed "left" subtrees ordered toward the root of the tree.
    std::vector<boost::optional<Hash>> parents;
    MerklePath path(std::deque<Hash> filler_hashes = std::deque<Hash>()) const;
    Hash root(size_t depth, std::deque<Hash> filler_hashes = std::deque<Hash>()) const;
    bool is_complete(size_t depth = Depth) const;
    size_t next_depth(size_t skip) const;
    void wfcheck() const;
};

template<size_t Depth, typename Hash>
bool operator==(const IncrementalMerkleTree<Depth, Hash>& a,
                const IncrementalMerkleTree<Depth, Hash>& b) {
    return (a.emptyroots == b.emptyroots &&
            a.left == b.left &&
            a.right == b.right &&
            a.parents == b.parents);
}

template <size_t Depth, typename Hash>
class IncrementalWitness {
friend class IncrementalMerkleTree<Depth, Hash>;

public:
    // Required for Unserialize()
    IncrementalWitness() {}

    IncrementalWitness(const WitnessPack& pack) : tree(pack.tree) {
        filled.resize(pack.filled.size());
        for (size_t i = 0; i < filled.size(); i++) {
            filled[i].SetHex(pack.filled[i]);
        }

        IncrementalMerkleTree<Depth, Hash> cursor_tree(pack.cursor);
        if (cursor_tree.size() > 0) {
            cursor = std::move(cursor_tree);
        }

        cursor_depth = pack.cursor_depth;
    }

    MerklePath path() const {
        return tree.path(partial_path());
    }

    // Return the element being witnessed (should be a note
    // commitment!)
    Hash element() const {
        return tree.last();
    }

    uint64_t position() const {
        return tree.size() - 1;
    }

    Hash root() const {
        return tree.root(Depth, partial_path());
    }

    void append(Hash obj);

    template <size_t D, typename H>
    friend bool operator==(const IncrementalWitness<D, H>& a,
                           const IncrementalWitness<D, H>& b);

private:
    IncrementalMerkleTree<Depth, Hash> tree;
    std::vector<Hash> filled;
    boost::optional<IncrementalMerkleTree<Depth, Hash>> cursor;
    size_t cursor_depth = 0;
    std::deque<Hash> partial_path() const;
    IncrementalWitness(IncrementalMerkleTree<Depth, Hash> tree) : tree(tree) {}
};

template<size_t Depth, typename Hash>
bool operator==(const IncrementalWitness<Depth, Hash>& a,
                const IncrementalWitness<Depth, Hash>& b) {
    return (a.tree == b.tree &&
            a.filled == b.filled &&
            a.cursor == b.cursor &&
            a.cursor_depth == b.cursor_depth);
}

/*
class SHA256Compress : public uint256 {
public:
    SHA256Compress() : uint256() {}
    SHA256Compress(uint256 contents) : uint256(contents) { }

    static SHA256Compress combine(
        const SHA256Compress& a,
        const SHA256Compress& b,
        size_t depth
    );

    static SHA256Compress uncommitted() {
        return SHA256Compress();
    }
};*/

class PedersenHash : public zero::uint256 {
public:
    PedersenHash() : zero::uint256() {}
    PedersenHash(zero::uint256 contents) : zero::uint256(contents) { }

    static PedersenHash combine(
        const PedersenHash& a,
        const PedersenHash& b,
        size_t depth
    );

    static PedersenHash uncommitted();
};

template<size_t Depth, typename Hash>
EmptyMerkleRoots<Depth, Hash> IncrementalMerkleTree<Depth, Hash>::emptyroots;

} // end namespace `libzcash`

typedef libzcash::IncrementalMerkleTree<SAPLING_INCREMENTAL_MERKLE_TREE_DEPTH, libzcash::PedersenHash> SaplingMerkleTree;
typedef libzcash::IncrementalMerkleTree<INCREMENTAL_MERKLE_TREE_DEPTH_TESTING, libzcash::PedersenHash> SaplingTestingMerkleTree;

typedef libzcash::IncrementalWitness<SAPLING_INCREMENTAL_MERKLE_TREE_DEPTH, libzcash::PedersenHash> SaplingWitness;
typedef libzcash::IncrementalWitness<INCREMENTAL_MERKLE_TREE_DEPTH_TESTING, libzcash::PedersenHash> SaplingTestingWitness;

FC_REFLECT( libzcash::MerkleTreePack, (left)(right)(parents) )
FC_REFLECT( libzcash::WitnessPack, (tree)(filled)(cursor)(cursor_depth) )
FC_REFLECT( libzcash::MerklePathPack, (path_bytes)(index_int) )
#endif // _INCREMENTAL_MERKLE_TREE_H_ 

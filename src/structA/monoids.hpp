// src/structA/monoids.hpp
// Structure A, module 1 of 4: the aggregates (monoids) the segment tree is built over.
//
// A "Monoid policy" used by SegmentTree<Monoid> must provide:
//   using value_type = ...;                       // the aggregate type
//   static value_type identity();                 // e, with combine(e,x) == combine(x,e) == x
//   static value_type combine(const value_type&,  // associative operation (+)
//                             const value_type&);
//   static value_type from_value(const Raw&);     // lifts one raw array element
// and value_type must support operator== (used only by the invariant checker).
#ifndef STRUCTA_MONOIDS_HPP
#define STRUCTA_MONOIDS_HPP

#include <cassert>
#include <climits>
#include <numeric>
#include <string>

namespace structA {

// ---------------------------------------------------------------------------
// 1. Maximum with count of maximum
// ---------------------------------------------------------------------------

// Valid aggregates: count == 0 (the empty aggregate, max_val is ignored)
// or count >= 1 (a non-empty segment whose maximum is max_val).
// This is the carrier set M_mc of D1: {(-inf, 0)} U (V x Z>=1).
struct MaxCount {
    long long max_val;
    long long count;

    bool operator==(const MaxCount& other) const noexcept {
        if (count == 0 && other.count == 0) {
            return true;  // all empty aggregates are equal, whatever max_val holds
        }
        return max_val == other.max_val && count == other.count;
    }
    bool operator!=(const MaxCount& other) const noexcept { return !(*this == other); }
};

// DESIGN DECISION (identity without a numeric -infinity):
// No long long is smaller than LLONG_MIN, so -infinity cannot be a number.
// Instead the identity is stored as {LLONG_MIN, 0} and is recognised ONLY by
// count == 0. A real reading equal to LLONG_MIN is lifted to {LLONG_MIN, 1},
// so it can never be confused with the identity (see D1, Remark "Machine
// representation").
//
// Associativity (semantic argument, D1 Theorem 2.2): map a multiset S to
// f(S) = (max S, number of occurrences of max S). For disjoint S, T we have
// combine(f(S), f(T)) = f(S U T): the maximum of the union is the larger of the
// two maxima, and its count in the union is the sum of its counts in S and T
// (a side whose maximum is smaller contributes 0). Disjoint union is
// associative, hence so is combine.
//
// Counts add ONLY when both maxima are equal: if one maximum is strictly
// larger, the other side's elements are all smaller than it and do not contribute.
//
// Overflow: the count of a segment is at most its length, so count <= n <= 10^6.
struct MaxCountMonoid {
    using value_type = MaxCount;

    static value_type identity() noexcept { return value_type{LLONG_MIN, 0}; }

    static value_type from_value(long long v) noexcept { return value_type{v, 1}; }

    static value_type combine(const value_type& a, const value_type& b) noexcept {
        if (a.count == 0) return b;  // a is the empty aggregate
        if (b.count == 0) return a;  // b is the empty aggregate
        if (a.max_val > b.max_val) return a;
        if (b.max_val > a.max_val) return b;
        return value_type{a.max_val, a.count + b.count};  // tie: add counts
    }
};

// ---------------------------------------------------------------------------
// 2. Range gcd
// ---------------------------------------------------------------------------

// Domain: nonnegative long long. LLONG_MIN is NOT allowed (std::gcd on it is
// undefined behaviour); the interface promises nonnegative input.
// Identity: 0, because every integer divides 0, so gcd(x, 0) = gcd(0, x) = x
// for x >= 0, and gcd(0, 0) = 0.
// Associativity: gcd(a, gcd(b, c)) and gcd(gcd(a, b), c) both equal the
// greatest common divisor of the set {a, b, c}.
struct GcdMonoid {
    using value_type = long long;

    static value_type identity() noexcept { return 0LL; }

    static value_type from_value(long long v) noexcept {
        assert(v >= 0 && "GcdMonoid requires nonnegative inputs");  // removed by -DNDEBUG
        return v;
    }

    static value_type combine(value_type a, value_type b) { return std::gcd(a, b); }
};

// ---------------------------------------------------------------------------
// 3. Noncommutative test monoid (used only by tests)
// ---------------------------------------------------------------------------

// String concatenation. Identity: "" ("" + s == s + "" == s).
// Associative ((a+b)+c == a+(b+c)) but NOT commutative ("ab" != "ba"), so it
// detects any query that combines segments in the wrong order.
struct StringConcatMonoid {
    using value_type = std::string;

    static value_type identity() { return std::string(); }

    static value_type from_value(const std::string& s) { return s; }

    static value_type combine(const value_type& a, const value_type& b) { return a + b; }
};

}  // namespace structA

#endif  // STRUCTA_MONOIDS_HPP
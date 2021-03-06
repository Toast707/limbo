// vim:filetype=cpp:textwidth=120:shiftwidth=2:softtabstop=2:expandtab
// Copyright 2016-2017 Christoph Schwering
// Licensed under the MIT license. See LICENSE file in the project root.
//
// Solver implements limited belief implications. The key methods are Entails(),
// Determines(), and Consistent(), which determine whether the knowledge base
// consisting of the clauses added with AddClause() entails a query, determines
// a terms's denotation or is consistent with it, respectively. They are are
// sound but incomplete: if they return true, this answer is correct with
// respect to classical logic; if they return false, this may not be correct
// and should be rather interpreted as "don't know." The method
// EntailsComplete() uses Consistent() to implement a complete but unsound
// entailment relation. It is safe to call AddClause() between evaluating
// queries with Entails(), Determines(), EntailsComplete(), or Consistent().
//
// Splitting and assigning is done at a deterministic point, namely after
// reducing the outermost logical operators with conjunctive meaning (negated
// disjunction, double negation, negated existential). This is opposed to the
// original semantics from the KR-2016 paper where splitting can be done at any
// point during the reduction.
//
// Note that in the special case that the set of clauses can be shown to be
// inconsistent after the splits, Determines() returns the null term to indicate
// that [t=n] is entailed by the clauses for arbitrary n.
//
// In the original semantics, when a split sets (t = n), we also substitute n
// for t in the query to deal with nested terms. But since we often split before
// reducing quantifiers, t might occur later in the query only after quantifiers
// are reduced. Substituting n for t at splitting time is hence not sufficient.
// For that reason, we defer that substitution until the query is reduced to a
// clause for which subsumption is to be checked. Then we check for any nested
// term t in that clause whether its denotation is defined by a unit clause
// (t = n) in the setup, in which case we substitute n for t in the clause.
// Note that the unit clause does not need to come from a split. Hence we
// may even save some trivial splits, e.g., from [f(n) = n], [g(n) = n]
// we infer without split that [f(g(n)) = n].
//
// While the ECAI-2016 paper complements the sound but incomplete entailment
// relation with a complete but unsound entailment relation, Solver provides
// a sound but incomplete consistency check. It is easy to see that this is
// equivalent: Consistent(k, phi) == !EntailsComplete(k, Not(phi)) and
// EntailsComplete(k, phi)) == !Consistent(k, Not(phi)). The advantage of
// the Consistent() method is that it is perhaps less confusing and less prone
// to typos and shares some code with the sound Entails().

#ifndef LIMBO_SOLVER_H_
#define LIMBO_SOLVER_H_

#include <cassert>

#include <iterator>
#include <list>

#include <limbo/formula.h>
#include <limbo/grounder.h>
#include <limbo/literal.h>
#include <limbo/setup.h>
#include <limbo/term.h>

#include <limbo/internal/ints.h>
#include <limbo/internal/maybe.h>

namespace limbo {

class Solver {
 public:
  typedef Formula::split_level split_level;

  static constexpr bool kConsistencyGuarantee = true;
  static constexpr bool kNoConsistencyGuarantee = false;

  Solver(Symbol::Factory* sf, Term::Factory* tf) : tf_(tf), grounder_(sf, tf) {}
  Solver(const Solver&) = delete;
  Solver& operator=(const Solver&) = delete;
  Solver(Solver&&) = default;
  Solver& operator=(Solver&&) = default;

  void AddClause(const Clause& c) { grounder_.AddClause(c); }

  const Setup& setup() const { return grounder_.Ground(); }

  Grounder* grounder() { return &grounder_; }

  bool Entails(int k, const Formula& phi, bool assume_consistent) {
    assert(phi.objective());
    assert(phi.free_vars().empty());
    grounder_.PrepareForQuery(k, phi);
    const Setup& s = grounder_.Ground();
    TermSet split_terms =
      k == 0            ? TermSet() :
      assume_consistent ? grounder_.RelevantSplitTerms(phi) :
                          grounder_.SplitTerms();
    const SortedTermSet& names = grounder_.Names();
    return s.Subsumes(Clause{}) || ReduceConjunctions(s, split_terms, names, k, phi);
  }

  internal::Maybe<Term> Determines(int k, Term lhs, bool assume_consistent) {
    assert(lhs.primitive());
    grounder_.PrepareForQuery(k, lhs);
    const Setup& s = grounder_.Ground();
    TermSet split_terms =
      k == 0            ? TermSet() :
      assume_consistent ? grounder_.RelevantSplitTerms(lhs) :
                          grounder_.SplitTerms();
    const SortedTermSet& names = grounder_.Names();
    internal::Maybe<Term> inconsistent_result = internal::Just(Term());
    internal::Maybe<Term> unsuccessful_result = internal::Nothing;
    return Split<true>(s, split_terms, names, k,
                       [this, &names, lhs](const Setup& s) { return s.Determines(lhs); },
                       [](internal::Maybe<Term> r1, internal::Maybe<Term> r2) {
                         return r1 && r2 && r1.val == r2.val ? r1 :
                                r1 && r2 && r1.val.null()    ? r2 :
                                r1 && r2 && r2.val.null()    ? r1 :
                                                               internal::Nothing;
                       },
                       inconsistent_result, unsuccessful_result);
  }

  bool EntailsComplete(int k, const Formula& phi, bool assume_consistent) {
    assert(phi.objective());
    assert(phi.free_vars().empty());
    Formula::Ref psi = Formula::Factory::Not(phi.Clone());
    return !Consistent(k, *psi, assume_consistent);
  }

  bool Consistent(int k, const Formula& phi, bool assume_consistent) {
    assert(phi.objective());
    assert(phi.free_vars().empty());
    grounder_.PrepareForQuery(k, phi);
    const Setup& s = grounder_.Ground();
    LiteralAssignmentSet assign_lits =
      k == 0            ? LiteralAssignmentSet() :
      assume_consistent ? grounder_.RelevantLiteralAssignments(phi) :
                          grounder_.LiteralAssignments();
    TermSet relevant_terms =
      assume_consistent ? grounder_.RelevantSplitTerms(phi) :
                          TermSet();
    const SortedTermSet& names = grounder_.Names();
    return ReduceDisjunctions(s, assign_lits, names, k, phi, assume_consistent, relevant_terms);
  }

 private:
#ifdef FRIEND_TEST
  FRIEND_TEST(SolverTest, Constants);
#endif

  typedef Grounder::TermSet TermSet;
  typedef Grounder::LiteralSet LiteralSet;
  typedef Grounder::LiteralAssignmentSet LiteralAssignmentSet;
  typedef Grounder::SortedTermSet SortedTermSet;

  bool ReduceConjunctions(const Setup& s,
                          const TermSet& split_terms,
                          const SortedTermSet& names,
                          int k,
                          const Formula& phi) {
    assert(phi.objective());
    switch (phi.type()) {
      case Formula::kNot: {
        switch (phi.as_not().arg().type()) {
          case Formula::kAtomic: {
            const Clause c = phi.as_not().arg().as_atomic().arg();
            return std::all_of(c.begin(), c.end(), [&, this](Literal a) {
              a = a.flip();
              Formula::Ref psi = Formula::Factory::Atomic(Clause{a});
              return ReduceConjunctions(s, split_terms, names, k, *psi);
            });
          }
          case Formula::kNot: {
            return ReduceConjunctions(s, split_terms, names, k, phi.as_not().arg().as_not().arg());
          }
          case Formula::kOr: {
            Formula::Ref left = Formula::Factory::Not(phi.as_not().arg().as_or().lhs().Clone());
            Formula::Ref right = Formula::Factory::Not(phi.as_not().arg().as_or().rhs().Clone());
            return ReduceConjunctions(s, split_terms, names, k, *left) &&
                   ReduceConjunctions(s, split_terms, names, k, *right);
          }
          case Formula::kExists: {
            const Term x = phi.as_not().arg().as_exists().x();
            const Formula& psi = phi.as_not().arg().as_exists().arg();
            const TermSet& ns = names[x.sort()];
            return std::all_of(ns.begin(), ns.end(), [&, this](const Term n) {
              Formula::Ref xi = Formula::Factory::Not(psi.Clone());
              xi->SubstituteFree(Term::Substitution(x, n), tf_);
              return ReduceConjunctions(s, split_terms, names, k, *xi);
            });
          }
          default:
            break;
        }
      }
      default: {
        if (phi.trivially_valid()) {
          return true;
        }
        return Split<false>(s, split_terms, names, k,
                            [this, &names, &phi](const Setup& s) { return Reduce(s, names, phi); },
                            [](bool r1, bool r2) { return r1 && r2; },
                            true, false);
      }
    }
    throw;
  }

  bool ReduceDisjunctions(const Setup& s,
                          const LiteralAssignmentSet& assign_lits,
                          const SortedTermSet& names,
                          int k,
                          const Formula& phi,
                          bool assume_consistent,
                          const TermSet& relevant_terms) {
    assert(phi.objective());
    switch (phi.type()) {
      case Formula::kAtomic: {
        return Assign(s, assign_lits, names, k, phi, assume_consistent, relevant_terms);
      }
      case Formula::kOr: {
        const Formula& left = phi.as_or().lhs();
        const Formula& right = phi.as_or().rhs();
        return ReduceDisjunctions(s, assign_lits, names, k, left, assume_consistent, relevant_terms) ||
               ReduceDisjunctions(s, assign_lits, names, k, right, assume_consistent, relevant_terms);
      }
      case Formula::kExists: {
        const Term x = phi.as_exists().x();
        const TermSet& ns = names[x.sort()];
        return std::any_of(ns.begin(), ns.end(), [&, this](const Term n) {
          Formula::Ref psi = phi.as_exists().arg().Clone();
          psi->SubstituteFree(Term::Substitution(x, n), tf_);
          return ReduceDisjunctions(s, assign_lits, names, k, *psi, assume_consistent, relevant_terms);
        });
      }
      case Formula::kNot: {
        switch (phi.as_not().arg().type()) {
          case Formula::kNot:
            return ReduceDisjunctions(s, assign_lits, names, k, phi.as_not().arg().as_not().arg(), assume_consistent,
                                      relevant_terms);
          default:
            return !phi.trivially_invalid() && Assign(s, assign_lits, names, k, phi, assume_consistent, relevant_terms);
        }
      }
      case Formula::kKnow:
      case Formula::kCons:
      case Formula::kBel:
      case Formula::kGuarantee:
        assert(false);
        return false;
    }
    throw;
  }

  bool Reduce(const Setup& s, const SortedTermSet& names, const Formula& phi) {
    assert(phi.objective());
    switch (phi.type()) {
      case Formula::kAtomic: {
        const Clause c = phi.as_atomic().arg();
        assert(c.ground());
        assert(c.valid() || c.primitive());
        return s.Subsumes(c);
      }
      case Formula::kNot: {
        switch (phi.as_not().arg().type()) {
          case Formula::kAtomic: {
            const Clause c = phi.as_not().arg().as_atomic().arg();
            return std::all_of(c.begin(), c.end(), [this, &s, &names](Literal a) {
              Formula::Ref psi = Formula::Factory::Atomic(Clause{a.flip()});
              return Reduce(s, names, *psi);
            });
          }
          case Formula::kNot: {
            return Reduce(s, names, phi.as_not().arg().as_not().arg());
          }
          case Formula::kOr: {
            Formula::Ref left = Formula::Factory::Not(phi.as_not().arg().as_or().lhs().Clone());
            Formula::Ref right = Formula::Factory::Not(phi.as_not().arg().as_or().rhs().Clone());
            return Reduce(s, names, *left) &&
                   Reduce(s, names, *right);
          }
          case Formula::kExists: {
            const Term x = phi.as_not().arg().as_exists().x();
            const Formula& psi = phi.as_not().arg().as_exists().arg();
            const TermSet& ns = names[x.sort()];
            return std::all_of(ns.begin(), ns.end(), [this, &s, &names, &psi, x](const Term n) {
              Formula::Ref xi = Formula::Factory::Not(psi.Clone());
              xi->SubstituteFree(Term::Substitution(x, n), tf_);
              return Reduce(s, names, *xi);
            });
          }
          case Formula::kKnow:
          case Formula::kCons:
          case Formula::kBel:
          case Formula::kGuarantee:
            assert(false);
            break;
        }
      }
      case Formula::kOr: {
        const Formula& left = phi.as_or().lhs();
        const Formula& right = phi.as_or().rhs();
        return Reduce(s, names, left) ||
               Reduce(s, names, right);
      }
      case Formula::kExists: {
        const Term x = phi.as_exists().x();
        const Formula& psi = phi.as_exists().arg();
        const TermSet& ns = names[x.sort()];
        return std::any_of(ns.begin(), ns.end(), [this, &s, &names, &psi, x](const Term n) {
          Formula::Ref xi = psi.Clone();
          xi->SubstituteFree(Term::Substitution(x, n), tf_);
          return Reduce(s, names, *xi);
        });
      }
      case Formula::kKnow:
      case Formula::kCons:
      case Formula::kBel:
      case Formula::kGuarantee:
        assert(false);
        return false;
    }
    throw;
  }

  template<bool split_order_matters, typename T, typename GoalPredicate, typename MergeResultPredicate>
  T Split(const Setup& s,
          const TermSet& split_terms,
          const SortedTermSet& names,
          int k,
          GoalPredicate goal,
          MergeResultPredicate merge,
          T inconsistent_result,
          T unsuccessful_result) {
    return Split<split_order_matters>(s, split_terms.begin(), split_terms.end(), split_terms.size(), names, k,
                                      goal, merge, inconsistent_result, unsuccessful_result);
  }

  template<bool split_order_matters, typename T, typename GoalPredicate, typename MergeResultPredicate>
  T Split(const Setup& s,
          const TermSet::const_iterator split_terms_begin,
          const TermSet::const_iterator split_terms_end,
          const internal::size_t n_split_terms,
          const SortedTermSet& names,
          int k,
          GoalPredicate goal,
          MergeResultPredicate merge,
          T inconsistent_result,
          T unsuccessful_result) {
    // For Determines(), the split order matters, for Entails() it does not.
    // Suppose we have two split terms t1, t2, t3 and two names n1, n2, and
    // a query term t and two candidate names n, n' for t.
    //
    // Let us assume that for all combinations of t1=N*, t3=N**, the reasoner
    // finds that t=n.
    //
    // The reasoner splits t1 at the first level, and after setting t1=n1 it
    // descends to the next split level, where it successfully splits t2, which
    // obtains t=n' as binding for t. Back at split level one, the reasoner now
    // considers the case t1=n2, again descends to the next level, where it
    // splits, say, t3, and obtains t=n.
    //
    // Back at split level one, the reasoner sees that t=n' and t=n are
    // incompatible and hence proceeds by splitting t2, which does not succeed.
    // In a way, t=n' found with t2 after t1=n1 blocks the real solution.
    //
    // Again at level one, the reasoner splits t3. If the order does not
    // matter, it will not descend to any further split level, since all
    // unordered combinations of t1, t2, t3 have been tested already.
    //
    // If the order does matter, however, it does descend to the next level.
    // There it will split t1: even if it picks t2 before t1, t2 will prove
    // incompatible with t3 (*). And once it splits t1 at level two, it will
    // find t=n, which this time is not blocked by t=n'.
    //
    // (*) This holds under the assumption that the KB is consistent.
    // If the KB is not consistent, could split terms 'block' each other?
    assert(std::distance(split_terms_begin, split_terms_end) == n_split_terms);
    if (s.contains_empty_clause()) {
      return unsuccessful_result;
    } else if (k > 0 && n_split_terms > 0) {
      assert(split_terms_begin != split_terms_end);
      internal::size_t n_split_terms_left = n_split_terms;
      bool recursed = false;
      for (auto it = split_terms_begin; it != split_terms_end; ) {
        if (n_split_terms >= k && n_split_terms_left < k-1) {
          break;
        }
        const Term t = *it++;
        --n_split_terms_left;
        if (s.Determines(t)) {
          continue;
        }
        auto merged_result = unsuccessful_result;
        const TermSet& ns = names[t.sort()];
        assert(!ns.empty());
        for (const Term n : ns) {
          Setup::ShallowCopy split_setup = s.shallow_copy();
          const Setup::Result add_result = split_setup.AddUnit(Literal::Eq(t, n));
          if (add_result == Setup::kInconsistent) {
            merged_result = !merged_result ? inconsistent_result : merge(merged_result, inconsistent_result);
            if (!merged_result) {
              goto next_split;
            }
            recursed = true;
            continue;
          }
          auto split_result = Split<split_order_matters>(*split_setup,
                                                         split_order_matters ? split_terms_begin : it,
                                                         split_terms_end,
                                                         split_order_matters ? n_split_terms : n_split_terms_left,
                                                         names,
                                                         k - 1,
                                                         goal,
                                                         merge,
                                                         inconsistent_result,
                                                         unsuccessful_result);
          if (!split_result) {
            goto next_split;
          }
          merged_result = !merged_result ? split_result : merge(merged_result, split_result);
          if (!merged_result) {
            goto next_split;
          }
          recursed = true;
        }
        return merged_result;
next_split:
        {}
      }
      return recursed ? unsuccessful_result : goal(s);
    } else {
      return goal(s);
    }
  }

  bool Assign(const Setup& s,
              const LiteralAssignmentSet& assign_lits,
              const SortedTermSet& names,
              int k,
              const Formula& phi,
              bool assume_consistent,
              const TermSet&  relevant_terms) {
    assert(phi.objective());
    if ((!assume_consistent && s.Subsumes(Clause{})) || phi.trivially_invalid()) {
      return false;
    } else if (k > 0 && !assign_lits.empty()) {
      if (assign_lits.empty()) {
        assert(phi.trivially_valid() || phi.trivially_invalid());
        return phi.trivially_valid();
      }
      assert(!assign_lits.empty());
      return std::any_of(assign_lits.begin(), assign_lits.end(), [&](const LiteralSet& lits) {
        assert(!lits.empty());
        Setup::ShallowCopy split_setup = s.shallow_copy();
        for (Literal a : lits) {
          if (!s.Subsumes(Clause{a.flip()})) {
            split_setup.AddUnit(a);
          }
        }
        return Assign(*split_setup, assign_lits, names, k-1, phi, assume_consistent, relevant_terms);
      });
    } else {
      if (!assume_consistent && !s.Consistent()) {
        return false;
        // TODO XXX There's something wrong with how we use
        // Setup::LocallyConsistent() here. The argument should rather be the
        // grounded terms from the query. And Setup::LocallyConsistent() should
        // close the set only under unsubsumed clauses.
      } else if (assume_consistent && !s.LocallyConsistent(relevant_terms)) {
        return false;
      }
      return Reduce(s, names, phi);
    }
  }

  Term::Factory* tf_;
  Grounder grounder_;
};

}  // namespace limbo

#endif  // LIMBO_SOLVER_H_


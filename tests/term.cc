// vim:filetype=cpp:textwidth=120:shiftwidth=2:softtabstop=2:expandtab
// Copyright 2016 Christoph Schwering

#include <gtest/gtest.h>

#include <lela/term.h>
#include <lela/format/output.h>

namespace lela {

using namespace lela::format;

struct EqSubstitute {
  EqSubstitute(Term pre, Term post) : pre_(pre), post_(post) {}
  internal::Maybe<Term> operator()(Term t) const { if (t == pre_) return internal::Just(post_); else return internal::Nothing; }

 private:
  const Term pre_;
  const Term post_;
};

TEST(TermTest, general) {
  Symbol::Factory& sf = *Symbol::Factory::Instance();
  Term::Factory& tf = *Term::Factory::Instance();
  const Symbol::Sort s1 = sf.CreateSort();
  const Symbol::Sort s2 = sf.CreateSort();
  EXPECT_EQ(s1, s1);
  EXPECT_EQ(s2, s2);
  EXPECT_TRUE(s1 != s2);

  const Term n1 = tf.CreateTerm(Symbol::Factory::CreateName(1, s1, 0));
  const Term n2 = tf.CreateTerm(Symbol::Factory::CreateName(2, s1, 0));
  EXPECT_TRUE(n1 == tf.CreateTerm(Symbol::Factory::CreateName(1, s1, 0)) && n2 != tf.CreateTerm(Symbol::Factory::CreateName(1, s1, 0)));
  EXPECT_TRUE(n1 != tf.CreateTerm(Symbol::Factory::CreateName(2, s1, 0)) && n2 == tf.CreateTerm(Symbol::Factory::CreateName(2, s1, 0)));
  EXPECT_TRUE(!n1.null() && n1.name() && !n1.variable() && !n1.function());
  EXPECT_TRUE(!n2.null() && n2.name() && !n2.variable() && !n2.function());
  EXPECT_EQ(n1.symbol().id(), 1);
  EXPECT_EQ(n2.symbol().id(), 2);

  const Term x1 = tf.CreateTerm(Symbol::Factory::CreateVariable(1, s1));
  const Term x2 = tf.CreateTerm(Symbol::Factory::CreateVariable(2, s1));
  EXPECT_TRUE(!x1.null() && !x1.name() && x1.variable() && !x1.function());
  EXPECT_TRUE(!x2.null() && !x2.name() && x2.variable() && !x2.function());
  EXPECT_TRUE(n1 != x1 && n1 != x2 && n2 != x1 && n2 != x2);
  EXPECT_TRUE(x1 == tf.CreateTerm(Symbol::Factory::CreateVariable(1, s1)) && x2 != tf.CreateTerm(Symbol::Factory::CreateVariable(1, s1)));
  EXPECT_TRUE(x1 != tf.CreateTerm(Symbol::Factory::CreateVariable(2, s1)) && x2 == tf.CreateTerm(Symbol::Factory::CreateVariable(2, s1)));
  EXPECT_EQ(x1.symbol().id(), 1);
  EXPECT_EQ(x2.symbol().id(), 2);

  const Term f1 = tf.CreateTerm(Symbol::Factory::CreateFunction(1, s1, 1), {n1});
  const Term f2 = tf.CreateTerm(Symbol::Factory::CreateFunction(2, s2, 2), {n1,x2});
  const Term f3 = tf.CreateTerm(Symbol::Factory::CreateFunction(1, s2, 1), {f1});
  const Term f4 = tf.CreateTerm(Symbol::Factory::CreateFunction(2, s2, 2), {n1,f1});
  EXPECT_TRUE(!f1.null() && !f1.name() && !f1.variable() && f1.function() && f1.ground() && f1.primitive() && f1.quasiprimitive());
  EXPECT_TRUE(!f2.null() && !f2.name() && !f2.variable() && f2.function() && !f2.ground() && !f2.primitive() && f2.quasiprimitive());
  EXPECT_TRUE(!f3.null() && !f3.name() && !f3.variable() && f3.function() && f3.ground() && !f3.primitive() && !f3.quasiprimitive());
  EXPECT_TRUE(!f4.null() && !f4.name() && !f4.variable() && f4.function() && f4.ground() && !f4.primitive() && !f4.quasiprimitive());
  const Term f5 = f2.Substitute(EqSubstitute(x2, f1), &tf);
  EXPECT_TRUE(f2 != f4);
  EXPECT_TRUE(!f5.name() && !f5.variable() && f5.function() && f5.ground() && !f4.primitive() && !f4.quasiprimitive());
  EXPECT_TRUE(f5 != f2);
  EXPECT_TRUE(f5 == f4);
  EXPECT_TRUE(f5 == tf.CreateTerm(Symbol::Factory::CreateFunction(2, s2, 2), {n1,f1}));
  EXPECT_EQ(f1.symbol().id(), 1);
  EXPECT_EQ(f2.symbol().id(), 2);
  EXPECT_EQ(f3.symbol().id(), 1);
  EXPECT_EQ(f4.symbol().id(), 2);

  typedef std::unordered_set<Term> TermSet;
  TermSet terms;

  terms.clear();
  f4.Traverse([&terms, s1](const Term t) { if (t.symbol().sort() == s1) { terms.insert(t); } return true; });
  EXPECT_TRUE(terms == TermSet({f1,n1}));

  terms.clear();
  f4.Traverse([&terms](const Term t) { terms.insert(t); return true; });
  EXPECT_TRUE(terms == TermSet({n1,f1,f4}));

  std::set<Symbol::Sort> sorts;
  f4.Traverse([&sorts](Term t) { sorts.insert(t.symbol().sort()); return true; });
  EXPECT_TRUE(sorts == std::set<Symbol::Sort>({s1,s2}));
}

}  // namespace lela


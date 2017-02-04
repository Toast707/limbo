// vim:filetype=cpp:textwidth=120:shiftwidth=2:softtabstop=2:expandtab
// Copyright 2016 Christoph Schwering

#include <gtest/gtest.h>

#include <lela/formula.h>
#include <lela/grounder.h>
#include <lela/format/output.h>

namespace lela {

using namespace lela::format::output;

template<typename T>
size_t length(T r) { return std::distance(r.begin(), r.end()); }

TEST(GrounderTest, Ground_SplitTerms_Names) {
  Symbol::Factory& sf = *Symbol::Factory::Instance();
  Term::Factory& tf = *Term::Factory::Instance();
  const Symbol::Sort s1 = sf.CreateSort();                  RegisterSort(s1, "");
  const Symbol::Sort s2 = sf.CreateSort();                  RegisterSort(s2, "");
  const Term n1 = tf.CreateTerm(sf.CreateName(s1, 0));      RegisterSymbol(n1.symbol(), "n1");
  const Term n2 = tf.CreateTerm(sf.CreateName(s1, 0));      RegisterSymbol(n2.symbol(), "n2");
  const Term n3 = tf.CreateTerm(sf.CreateName(s2, 0));      RegisterSymbol(n3.symbol(), "n3");
  const Term x1 = tf.CreateTerm(sf.CreateVariable(s1));     RegisterSymbol(x1.symbol(), "x1");
  const Term x2 = tf.CreateTerm(sf.CreateVariable(s1));     RegisterSymbol(x2.symbol(), "x2");
  const Term x3 = tf.CreateTerm(sf.CreateVariable(s2));     RegisterSymbol(x3.symbol(), "x3");
  //const Term x4 = tf.CreateTerm(sf.CreateVariable(s2));     RegisterSymbol(x4, "x4");
  const Symbol a = sf.CreateFunction(s1, 0);                RegisterSymbol(a, "a");
  const Symbol f = sf.CreateFunction(s1, 1);                RegisterSymbol(f, "f");
  const Symbol g = sf.CreateFunction(s2, 1);                RegisterSymbol(g, "g");
  const Symbol h = sf.CreateFunction(s2, 2);                RegisterSymbol(h, "h");
  //const Symbol i = sf.CreateFunction(s2, 2);
  //const Term c1 = tf.CreateTerm(a, {});
  //const Term f1 = tf.CreateTerm(f, {n1});
  //const Term f2 = tf.CreateTerm(h, {n1,x2});
  //const Term f3 = tf.CreateTerm(g, {f1});
  //const Term f4 = tf.CreateTerm(h, {n1,f1});
  //const Term f5 = tf.CreateTerm(i, {x1,x3});

#if 0 // Disabled because:
      // We allow only quasi-primitive formulas to be grounded.
      // (Actually, after Clause::Minimize() some of the following clauses are
      // quasi-primitive; e.g., [n1/=n1] is reduced to [].)
  {
    Grounder g(&sf, &tf);
    g.AddClause(Clause({Literal::Eq(n1,n1)}));
    const lela::Setup& s = g.Ground();
    // Grounding should be [n1=n1]. The clause is valid and hence skipped.
    EXPECT_EQ(length(s.clauses()), 0);
    EXPECT_TRUE(s.Consistent());
  }

  {
    Grounder g(&sf, &tf);
    g.AddClause(Clause({Literal::Neq(n1,n1)}));
    const lela::Setup& s = g.Ground();
    // Grounding should be [n1/=n1]. The clause is invalid and hence boiled
    // down to [].
    EXPECT_EQ(length(s.clauses()), 1);
    EXPECT_FALSE(s.Consistent());
  }

  {
    Grounder g(&sf, &tf);
    g.AddClause(Clause({Literal::Eq(x1, x1)}));
    const lela::Setup& s = g.Ground();
    // Grounding should be [n1=n1]. The clause is valid and hence skipped.
    EXPECT_EQ(length(s.clauses()), 0);
    EXPECT_TRUE(s.Consistent());
  }

  {
    Grounder g(&sf, &tf);
    g.AddClause(Clause({Literal::Neq(x1, x1)}));
    const lela::Setup& s = g.Ground();
    // Grounding should be [n1/=n1]. The clause is invalid and hence boiled
    // down to [].
    EXPECT_EQ(length(s.clauses()), 1);
    EXPECT_FALSE(s.Consistent());
  }

  {
    Grounder g(&sf, &tf);
    g.AddClause(Clause({Literal::Eq(n1, x1)}));
    const lela::Setup& s = g.Ground();
    // Grounding should be [n1=n1], [n3=n1]. The first clause is valid and
    // hence skipped. The second is invalid and hence boiled down to [].
    EXPECT_EQ(length(s.clauses()), 1);
    EXPECT_FALSE(s.Consistent());
  }

  {
    Grounder g(&sf, &tf);
    g.AddClause(Clause({Literal::Neq(n1, x1)}));
    const lela::Setup& s = g.Ground();
    // Grounding should be [n1/=n1], [n3/=n1]. The second clause is valid and
    // hence skipped. The first is invalid and hence boiled down to [].
    EXPECT_EQ(length(s.clauses()), 1);
    EXPECT_FALSE(s.Consistent());
  }

  {
    Grounder g(&sf, &tf);
    g.AddClause(Clause({Literal::Eq(x1, x2)}));
    const lela::Setup& s = g.Ground();
    // Grounding should be [n1=n1], [n3=2], [n1=n3], [n3=n1]. The former two
    // clauses are valid and hence skipped. The latter ones are invalid and
    // hence boiled down to [].
    EXPECT_EQ(length(s.clauses()), 1);
    EXPECT_FALSE(s.Consistent());
  }

  {
    Grounder g(&sf, &tf);
    g.AddClause(Clause({Literal::Neq(x1, x1)}));
    const lela::Setup& s = g.Ground();
    // Grounding should be [n1/=n1]. The clause is invalid and hence boiled
    // down to [].
    EXPECT_EQ(length(s.clauses()), 1);
    EXPECT_FALSE(s.Consistent());
  }
#endif

  {
    Grounder g(&sf, &tf);
    g.AddClause(Clause({Literal::Eq(tf.CreateTerm(a, {}), x1)}));
    const lela::Setup& s = g.Ground();
    // Grounding should be [a=n1].
    EXPECT_EQ(length(s.clauses()), 1);
    EXPECT_FALSE(s.Consistent());
  }

  {
    Grounder g(&sf, &tf);
    g.AddClause(Clause({Literal::Eq(tf.CreateTerm(f, {n1}), x1)}));
    const lela::Setup& s = g.Ground();
    // Grounding should be [f(n1)=n1)], [f(n1)=n3]. The clauses unify and
    // yield [].
    EXPECT_EQ(length(s.clauses()), 1);
    EXPECT_FALSE(s.Consistent());
  }

  {
    Grounder g(&sf, &tf);
    g.AddClause(Clause({Literal::Neq(tf.CreateTerm(f, {n1}), x2)}));
    const lela::Setup& s = g.Ground();
    // Grounding should be [f(n1)/=n1)], [f(n1)/=n3].
    EXPECT_EQ(length(s.clauses()), 3);
    EXPECT_TRUE(s.Consistent());
  }

  {
    Grounder g(&sf, &tf);
    g.AddClause(Clause({Literal::Eq(tf.CreateTerm(h, {n1,x2}), x3)}));
    const lela::Setup& s = g.Ground();
    // Grounding should be [h(n1,nX)=nY] for X=1,2,3 and Y=4,5. The clauses
    // unify and yield [].
    EXPECT_EQ(length(s.clauses()), 1);
    EXPECT_FALSE(s.Consistent());
  }

  {
    Grounder g(&sf, &tf);
    g.AddClause(Clause({Literal::Neq(tf.CreateTerm(h, {n1,x2}), x3)}));
    const lela::Setup& s = g.Ground();
    // Grounding should be [h(n1,nX)=nY] for X=1,2,3 and Y=4,5.
    EXPECT_EQ(length(s.clauses()), 3*2);
    EXPECT_TRUE(s.Consistent());
  }

//  {
//    Grounder g(&sf, &tf);
//    g.AddClause(Clause({Literal::Neq(c1, x1)}));
//    g.PrepareForQuery(Formula::Factory::Exists(x1, *Formula::Factory::Atomic({Literal::Eq(x1, x1)}))->NF(&sf, &tf));
//    g.PrepareForQuery(Formula::Factory::Exists(x2, *Formula::Factory::Atomic({Literal::Eq(x2, x2)}))->NF(&sf, &tf));
//    g.PrepareForQuery(Formula::Factory::Exists(x3, *Formula::Factory::Atomic({Literal::Eq(x3, x3)}))->NF(&sf, &tf));
//    g.PrepareForQuery(Formula::Factory::Exists(x4, *Formula::Factory::Atomic({Literal::Eq(x4, x4)}))->NF(&sf, &tf));
//    const lela::Setup& s = g.Ground();
//    EXPECT_EQ(length(s.clauses()), 2);
//  }

  {
    //auto phi = Formula::Factory::Exists(x3, Formula::Factory::Atomic({Literal::Eq(tf.CreateTerm(h, {n1,x3}), tf.CreateTerm(a, {}))}));
    auto phi = Formula::Factory::Exists(x3, Formula::Factory::Atomic({Literal::Eq(tf.CreateTerm(h, {n1,x3}), tf.CreateTerm(g, {tf.CreateTerm(a, {})}))}))->NF(&sf, &tf);
    // NF introduces two new variables of the same sort as x3, and one new of the same sort as n1.
    Grounder gg(&sf, &tf);
    gg.PrepareForQuery(1, *phi);
    Grounder::TermSet terms = gg.SplitTerms();
    Grounder::SortedTermSet names = gg.Names();
    //std::cout << phi << std::endl;
    //std::cout << names << std::endl;
    //std::cout << terms << std::endl;
    EXPECT_NE(x3.sort(), n1.sort());
    EXPECT_NE(x3.sort(), a.sort());
    EXPECT_EQ(names.size(), 2);
    EXPECT_EQ(n1.symbol().sort(), a.sort());
    EXPECT_EQ(x3.symbol().sort(), g.sort());
    EXPECT_EQ(x3.symbol().sort(), h.sort());
    EXPECT_EQ(names[n1.symbol().sort()].size(), 1+1+1);
    EXPECT_EQ(names[x3.symbol().sort()].size(), 0+2+1);
    Term na_1 = *names[a.sort()].begin();
    Term na_2 = *std::next(names[a.sort()].begin());
    Term na_3 = *std::next(std::next(names[a.sort()].begin()));
    EXPECT_TRUE(na_1 == n1 || na_2 == n1 || na_3 == n1);
    Term nx3_1 = *names[x3.sort()].begin();
    Term nx3_2 = *std::next(names[x3.sort()].begin());
    Term nx3_3 = *std::next(std::next(names[x3.sort()].begin()));
    EXPECT_TRUE(nx3_1 != nx3_2 && nx3_2 != nx3_3 && nx3_1 != nx3_3);
    EXPECT_EQ(std::set<Term>(terms.begin(), terms.end()),
              std::set<Term>({tf.CreateTerm(a, {}),
                             tf.CreateTerm(g, {n1}),
                             tf.CreateTerm(g, {na_1}),
                             tf.CreateTerm(g, {na_2}),
                             tf.CreateTerm(g, {na_3}),
                             tf.CreateTerm(h, {n1, nx3_1}),
                             tf.CreateTerm(h, {n1, nx3_2}),
                             tf.CreateTerm(h, {n1, nx3_3})}));
  }

  {
    Clause c{Literal::Eq(tf.CreateTerm(h, {n1,n3}), n3)}; 
    Clause d{Literal::Eq(tf.CreateTerm(h, {x1,n3}), n3)}; 
    Clause e{Literal::Eq(tf.CreateTerm(f, {x1}), n1)}; 
    auto phi = Formula::Factory::Exists(x3, Formula::Factory::Atomic({Literal::Eq(tf.CreateTerm(h, {n1,x3}), x3)}))->NF(&sf, &tf);
    Grounder g(&sf, &tf);
    const class Setup* last;
    {
      EXPECT_FALSE(g.names_changed_);
      EXPECT_EQ(g.unprocessed_clauses_.size(), 0);
      EXPECT_EQ(g.processed_clauses_.size(), 0);
      const class Setup* s = &g.Ground();
      EXPECT_EQ(length(s->clauses()), 0);
      EXPECT_EQ(g.setups_.size(), 1);
      last = s;
    }
    {
      EXPECT_FALSE(g.names_changed_);
      EXPECT_EQ(g.unprocessed_clauses_.size(), 0);
      EXPECT_EQ(g.processed_clauses_.size(), 0);
      const class Setup* s = &g.Ground();
      EXPECT_FALSE(g.names_changed_);
      EXPECT_EQ(g.unprocessed_clauses_.size(), 0);
      EXPECT_EQ(g.processed_clauses_.size(), 0);
      EXPECT_EQ(length(s->clauses()), 0);
      EXPECT_EQ(g.setups_.size(), 1);
      EXPECT_EQ(s, last);
      last = s;
    }
    g.AddClause(c);  // adds new name, re-ground everything
    {
      EXPECT_TRUE(g.names_changed_);
      EXPECT_EQ(g.unprocessed_clauses_.size(), 1);
      EXPECT_EQ(g.processed_clauses_.size(), 0);
      const class Setup* s = &g.Ground();
      EXPECT_FALSE(g.names_changed_);
      EXPECT_EQ(g.unprocessed_clauses_.size(), 0);
      EXPECT_EQ(g.processed_clauses_.size(), 1);
      EXPECT_EQ(length(s->clauses()), 1);
      EXPECT_EQ(g.setups_.size(), 1);
      last = s;
    }
    g.PrepareForQuery(0, *phi);  // adds new plus name, re-ground everything
    {
      EXPECT_TRUE(g.names_changed_);
      EXPECT_EQ(g.unprocessed_clauses_.size(), 0);
      EXPECT_EQ(g.processed_clauses_.size(), 1);
      const class Setup* s = &g.Ground();
      EXPECT_FALSE(g.names_changed_);
      EXPECT_EQ(g.unprocessed_clauses_.size(), 0);
      EXPECT_EQ(g.processed_clauses_.size(), 1);
      //EXPECT_NE(last, s);
      EXPECT_EQ(length(s->clauses()), 1);
      EXPECT_EQ(g.setups_.size(), 1);
      last = s;
    }
    g.AddClause(d);  // adds two new plus names (one for x, one for the Lemma 8 fix), re-ground everything
    {
      EXPECT_TRUE(g.names_changed_);
      EXPECT_EQ(g.unprocessed_clauses_.size(), 1);
      EXPECT_EQ(g.processed_clauses_.size(), 1);
      const class Setup* s = &g.Ground();
      EXPECT_FALSE(g.names_changed_);
      EXPECT_EQ(g.unprocessed_clauses_.size(), 0);
      EXPECT_EQ(g.processed_clauses_.size(), 2);
      //EXPECT_NE(last, s);
      EXPECT_EQ(length(s->clauses()), 3);
      EXPECT_EQ(g.setups_.size(), 1);
      last = s;
    }
    g.PrepareForQuery(1, *phi);  // adds no new plus name, 
    {
      EXPECT_FALSE(g.names_changed_);
      EXPECT_EQ(g.unprocessed_clauses_.size(), 0);
      EXPECT_EQ(g.processed_clauses_.size(), 2);
      const class Setup* s = &g.Ground();
      EXPECT_FALSE(g.names_changed_);
      EXPECT_EQ(g.unprocessed_clauses_.size(), 0);
      EXPECT_EQ(g.processed_clauses_.size(), 2);
      EXPECT_EQ(last, s);
      EXPECT_EQ(length(s->clauses()), 3);
      EXPECT_EQ(g.setups_.size(), 1);
      last = s;
    }
    g.AddClause(e);  // adds no new names
    {
      EXPECT_FALSE(g.names_changed_);
      EXPECT_EQ(g.unprocessed_clauses_.size(), 1);
      EXPECT_EQ(g.processed_clauses_.size(), 2);
      const class Setup* s = &g.Ground();
      EXPECT_FALSE(g.names_changed_);
      EXPECT_EQ(g.unprocessed_clauses_.size(), 0);
      EXPECT_EQ(g.processed_clauses_.size(), 3);
      EXPECT_NE(last, s);
      EXPECT_EQ(length(s->clauses()), 3+3);
      EXPECT_EQ(g.setups_.size(), 2);
      last = s;
    }
  }
}

TEST(GrounderTest, Assignments) {
  Symbol::Factory& sf = *Symbol::Factory::Instance();
  Term::Factory& tf = *Term::Factory::Instance();
  const Symbol::Sort s1 = sf.CreateSort();                  RegisterSort(s1, "");
  const Symbol::Sort s2 = sf.CreateSort();                  RegisterSort(s2, "");
  const Term n1 = tf.CreateTerm(sf.CreateName(s1, 0));      RegisterSymbol(n1.symbol(), "n1");
  const Term n2 = tf.CreateTerm(sf.CreateName(s1, 0));      RegisterSymbol(n2.symbol(), "n2");
  const Term n3 = tf.CreateTerm(sf.CreateName(s2, 0));      RegisterSymbol(n3.symbol(), "n3");
  const Term x1 = tf.CreateTerm(sf.CreateVariable(s1));     RegisterSymbol(x1.symbol(), "x1");
  const Term x2 = tf.CreateTerm(sf.CreateVariable(s1));     RegisterSymbol(x2.symbol(), "x2");
  const Term x3 = tf.CreateTerm(sf.CreateVariable(s2));     RegisterSymbol(x3.symbol(), "x3");
  //const Term x4 = tf.CreateTerm(sf.CreateVariable(s2));     RegisterSymbol(x4, "x4");
  //const Symbol a = sf.CreateFunction(s1, 0);                RegisterSymbol(a, "a");
  const Symbol f = sf.CreateFunction(s1, 1);                RegisterSymbol(f, "f");
  //const Symbol g = sf.CreateFunction(s2, 1);                RegisterSymbol(g, "g");
  //const Symbol h = sf.CreateFunction(s2, 2);                RegisterSymbol(h, "h");
  {
    Grounder::SortedTermSet ts;
    ts.insert(n1);
    Grounder::Assignments as({}, &ts);
    EXPECT_EQ(length(as), 1);
    Term fx1 = tf.CreateTerm(f, {x1});
    Term fn1 = tf.CreateTerm(f, {n1});
    Grounder::Assignments::Assignment a = *as.begin();
    EXPECT_EQ(fx1.Substitute(a, &tf), fx1);
    EXPECT_NE(fx1.Substitute(a, &tf), fn1);
  }
  {
    Grounder::SortedTermSet ts;
    ts.insert(n1);
    Grounder::Assignments as({x1}, &ts);
    EXPECT_EQ(length(as), 1);
    Term fx1 = tf.CreateTerm(f, {x1});
    Term fn1 = tf.CreateTerm(f, {n1});
    Grounder::Assignments::Assignment a = *as.begin();
    EXPECT_NE(fx1.Substitute(a, &tf), fx1);
    EXPECT_EQ(fx1.Substitute(a, &tf), fn1);
  }
  {
    Grounder::SortedTermSet ts;
    ts.insert(n1);
    ts.insert(n2);
    Grounder::Assignments as({x1}, &ts);
    EXPECT_EQ(length(as), 2);
    Term fx1 = tf.CreateTerm(f, {x1});
    Term fn1 = tf.CreateTerm(f, {n1});
    Term fn2 = tf.CreateTerm(f, {n2});
    Grounder::TermSet substitutes;
    Grounder::Assignments::Assignment a = *as.begin();
    substitutes.insert(fx1.Substitute(a, &tf));
    Grounder::Assignments::Assignment b = *std::next(as.begin());
    substitutes.insert(fx1.Substitute(b, &tf));
    EXPECT_EQ(substitutes.size(), 2);
    EXPECT_EQ(substitutes, Grounder::TermSet({fn1, fn2}));
    EXPECT_TRUE(substitutes.find(fx1) == substitutes.end());
  }
  {
    Grounder::SortedTermSet ts;
    ts.insert(n1);
    ts.insert(n2);
    ts.insert(n3);
    Grounder::Assignments as({x1,x2,x3}, &ts);
    EXPECT_EQ(length(as), 4);
  }
}

TEST(GrounderTest, Ground_SplitNames) {
  Symbol::Factory& sf = *Symbol::Factory::Instance();
  Term::Factory& tf = *Term::Factory::Instance();
  const Symbol::Sort Bool = sf.CreateSort();                            RegisterSort(Bool, "");
  const Symbol::Sort Human = sf.CreateSort();                           RegisterSort(Human, "");
  const Symbol::Sort Animal = sf.CreateSort();                          RegisterSort(Animal, "");
  //
  const Term T          = tf.CreateTerm(sf.CreateName(Bool, 0));        RegisterSymbol(T.symbol(), "T");
  //const Term F          = tf.CreateTerm(sf.CreateName(Bool, 0));
  //
  const Symbol IsHuman  = sf.CreateFunction(Bool, 1);                   RegisterSymbol(IsHuman, "IsHuman");
  const Term x          = tf.CreateTerm(sf.CreateVariable(Human));      RegisterSymbol(x.symbol(), "x");
  const Term xIsHuman   = tf.CreateTerm(IsHuman, {x});
  //
  const Symbol IsAnimal = sf.CreateFunction(Bool, 1);                   RegisterSymbol(IsAnimal, "IsAnimal");
  const Term a          = tf.CreateTerm(sf.CreateFunction(Animal, 0));  RegisterSymbol(a.symbol(), "a");
  const Term aIsAnimal  = tf.CreateTerm(IsAnimal, {a});
  //
  auto phi = Formula::Factory::Exists(x, Formula::Factory::Atomic(Clause({ Literal::Eq(xIsHuman, T), Literal::Neq(aIsAnimal, T) })))->NF(&sf, &tf);
  {
    Grounder g(&sf, &tf);
    g.PrepareForQuery(0, *phi);
    Grounder::SortedTermSet names = g.Names();
    EXPECT_EQ(names[Bool].size(), 1+1);
    EXPECT_EQ(names[Human].size(), 1+1);
    EXPECT_EQ(names[Animal].size(), 0+2);
    Grounder::TermSet terms = g.SplitTerms();
    EXPECT_EQ(terms.size(), 0);
  }
  {
    Grounder g(&sf, &tf);
    g.PrepareForQuery(1, *phi);
    Grounder::SortedTermSet names = g.Names();
    EXPECT_EQ(names[Bool].size(), 1+1);
    EXPECT_EQ(names[Human].size(), 1+1);
    EXPECT_EQ(names[Animal].size(), 0+2);
    Grounder::TermSet terms = g.SplitTerms();
    EXPECT_NE(terms.size(), 0);
  }
  {
    Grounder g(&sf, &tf);
    g.PrepareForQuery(2, *phi);
    Grounder::SortedTermSet names = g.Names();
    EXPECT_EQ(names[Bool].size(), 1+1);
    EXPECT_EQ(names[Human].size(), 1+1);
    EXPECT_EQ(names[Animal].size(), 0+2);
    Grounder::TermSet terms = g.SplitTerms();
    EXPECT_NE(terms.size(), 0);
  }
  {
    Grounder g(&sf, &tf);
    g.PrepareForQuery(3, *phi);
    Grounder::SortedTermSet names = g.Names();
    EXPECT_EQ(names[Bool].size(), 1+1);
    EXPECT_EQ(names[Human].size(), 1+1);
    EXPECT_EQ(names[Animal].size(), 0+2);
    Grounder::TermSet terms = g.SplitTerms();
    EXPECT_NE(terms.size(), 0);
  }
}

TEST(GrounderTest, Ground_SplitNames_iterated) {
  Symbol::Factory& sf = *Symbol::Factory::Instance();
  Term::Factory& tf = *Term::Factory::Instance();
  const Symbol::Sort Bool = sf.CreateSort();                            RegisterSort(Bool, "");
  const Symbol::Sort Human = sf.CreateSort();                           RegisterSort(Human, "");
  const Symbol::Sort Animal = sf.CreateSort();                          RegisterSort(Animal, "");
  //
  const Term T          = tf.CreateTerm(sf.CreateName(Bool, 0));        RegisterSymbol(T.symbol(), "T");
  //const Term F          = tf.CreateTerm(sf.CreateName(Bool, 0));
  //
  const Symbol IsHuman  = sf.CreateFunction(Bool, 1);                   RegisterSymbol(IsHuman, "IsHuman");
  const Term x          = tf.CreateTerm(sf.CreateVariable(Human));      RegisterSymbol(x.symbol(), "x");
  const Term xIsHuman   = tf.CreateTerm(IsHuman, {x});
  //
  const Symbol IsAnimal = sf.CreateFunction(Bool, 1);                   RegisterSymbol(IsAnimal, "IsAnimal");
  const Term a          = tf.CreateTerm(sf.CreateFunction(Animal, 0));  RegisterSymbol(a.symbol(), "a");
  const Term aIsAnimal  = tf.CreateTerm(IsAnimal, {a});
  //
  auto phi = Formula::Factory::Exists(x, Formula::Factory::Atomic(Clause({ Literal::Eq(xIsHuman, T), Literal::Neq(aIsAnimal, T) })))->NF(&sf, &tf);
  // same as previous test except that we re-use the grounder
  Grounder g(&sf, &tf);
  {
    g.PrepareForQuery(0, *phi);
    Grounder::SortedTermSet names = g.Names();
    EXPECT_EQ(names[Bool].size(), 1+1);
    EXPECT_EQ(names[Human].size(), 1+1);
    EXPECT_EQ(names[Animal].size(), 0+2);
    Grounder::TermSet terms = g.SplitTerms();
    EXPECT_EQ(terms.size(), 0);
  }
  {
    g.PrepareForQuery(1, *phi);
    Grounder::SortedTermSet names = g.Names();
    EXPECT_EQ(names[Bool].size(), 1+1);
    EXPECT_EQ(names[Human].size(), 1+1);
    EXPECT_EQ(names[Animal].size(), 0+2);
    Grounder::TermSet terms = g.SplitTerms();
    EXPECT_NE(terms.size(), 0);
  }
  {
    g.PrepareForQuery(2, *phi);
    Grounder::SortedTermSet names = g.Names();
    EXPECT_EQ(names[Bool].size(), 1+1);
    EXPECT_EQ(names[Human].size(), 1+1);
    EXPECT_EQ(names[Animal].size(), 0+2);
    Grounder::TermSet terms = g.SplitTerms();
    EXPECT_NE(terms.size(), 0);
  }
  {
    g.PrepareForQuery(3, *phi);
    Grounder::SortedTermSet names = g.Names();
    EXPECT_EQ(names[Bool].size(), 1+1);
    EXPECT_EQ(names[Human].size(), 1+1);
    EXPECT_EQ(names[Animal].size(), 0+2);
    Grounder::TermSet terms = g.SplitTerms();
    EXPECT_NE(terms.size(), 0);
  }
}

}  // namespace lela


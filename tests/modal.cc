// vim:filetype=cpp:textwidth=120:shiftwidth=2:softtabstop=2:expandtab
// Copyright 2016 Christoph Schwering

#include <gtest/gtest.h>

#include <lela/modal.h>
#include <lela/format/output.h>
#include <lela/format/cpp/syntax.h>

namespace lela {

using namespace lela::format::cpp;
using namespace lela::format::output;

#define REGISTER_SYMBOL(x)    RegisterSymbol(x, #x)

inline void RegisterSymbol(Term t, const std::string& n) {
  RegisterSymbol(t.symbol(), n);
}

TEST(SolverTest, ECAI2016Sound) {
  Context ctx;
  KnowledgeBase kb(ctx.sf(), ctx.tf());
  auto Bool = ctx.CreateSort();                   RegisterSort(Bool, "");
  auto Food = ctx.CreateSort();                   RegisterSort(Food, "");
  auto T = ctx.CreateName(Bool, 0)();             REGISTER_SYMBOL(T);
  auto Aussie = ctx.CreateFunction(Bool, 0)();    REGISTER_SYMBOL(Aussie);
  auto Italian = ctx.CreateFunction(Bool, 0)();   REGISTER_SYMBOL(Italian);
  auto Eats = ctx.CreateFunction(Bool, 1);        REGISTER_SYMBOL(Eats);
  auto Meat = ctx.CreateFunction(Bool, 1);        REGISTER_SYMBOL(Meat);
  auto Veggie = ctx.CreateFunction(Bool, 0)();    REGISTER_SYMBOL(Veggie);
  auto roo = ctx.CreateName(Food, 0)();           REGISTER_SYMBOL(roo);
  auto x = ctx.CreateVariable(Food);              REGISTER_SYMBOL(x);
  Formula::split_level k = 1;
  Formula::split_level l = 1;
  EXPECT_TRUE(kb.Add(*Formula::Factory::Bel(k, l, *(Aussie == T), *(Italian != T))));
  EXPECT_TRUE(kb.Add(*Formula::Factory::Bel(k, l, *(Italian == T), *(Aussie != T))));
  EXPECT_TRUE(kb.Add(*Formula::Factory::Bel(k, l, *(Aussie == T), *(Eats(roo) == T))));
  EXPECT_TRUE(kb.Add(*Formula::Factory::Bel(k, l, *(T == T), *(Italian == T || Veggie == T))));
  EXPECT_TRUE(kb.Add(*Formula::Factory::Bel(k, l, *(Italian != T), *(Aussie == T))));
  EXPECT_TRUE(kb.Add(*Formula::Factory::Bel(k, l, *(Meat(roo) != T), *(T != T))));
  EXPECT_TRUE(kb.Add(*Formula::Factory::Bel(k, l, *(~Fa(x, (Veggie == T && Meat(x) == T) >> (Eats(x) != T))), *(T != T))));
  EXPECT_FALSE(kb.Entails(*Formula::Factory::Bel(0, 0, *(Italian != T), *(Veggie != T))));
  EXPECT_FALSE(kb.Entails(*Formula::Factory::Bel(0, 1, *(Italian != T), *(Veggie != T))));
  EXPECT_FALSE(kb.Entails(*Formula::Factory::Bel(1, 0, *(Italian != T), *(Veggie != T))));
  EXPECT_TRUE(kb.Entails(*Formula::Factory::Bel(1, 1, *(Italian != T), *(Veggie != T))));
}

}  // namespace lela


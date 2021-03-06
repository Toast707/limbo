// vim:filetype=cpp:textwidth=120:shiftwidth=2:softtabstop=2:expandtab
// Copyright 2014 Christoph Schwering

#ifndef EXAMPLES_MINESWEEPER_AGENT_H_
#define EXAMPLES_MINESWEEPER_AGENT_H_

#include <algorithm>
#include <iostream>

#include "game.h"
#include "kb.h"

template<typename Logger>
class Agent {
 public:
  explicit Agent(Game* g, KnowledgeBase* kb, Logger logger = Logger()) : g_(g), kb_(kb), logger_(logger) {}

  int Explore() {
    kb_->Sync();

    // First open a random point which is not at the edge of the field.
    if (g_->n_opens() == 0) {
      Point p;
      do {
        p = g_->RandomPoint();
      } while (g_->neighbors_of(p).size() < 8);
      logger_.explored(p, -1);
      g_->OpenWithFrontier(p);
      last_point_ = p;
      return -1;
    }

    // First look for a field which is known not to be a mine.
    for (size_t k = 0; k <= kb_->max_k(); ++k) {
      std::vector<bool> inspected(g_->n_fields(), false);
      const std::size_t max_radius = std::max(std::max(last_point_.x, g_->width() - last_point_.x),
                                              std::max(last_point_.y, g_->height() - last_point_.y));
      for (std::size_t radius = 0; radius <= max_radius; ++radius) {
        std::vector<bool> on_rectangle = Rectangle(last_point_, radius);
        for (std::size_t i = 0; i < g_->n_fields(); ++i) {
          if (inspected[i] || !on_rectangle[i]) {
            continue;
          }
          const Point p = g_->to_point(i);
          if (g_->opened(p) || g_->flagged(p)) {
            continue;
          }
          const limbo::internal::Maybe<bool> r = kb_->IsMine(p, k);
          if (r.yes) {
            if (r.val) {
              logger_.flagged(p, k);
              g_->Flag(p);
            } else {
              logger_.explored(p, k);
              g_->OpenWithFrontier(p);
            }
            last_point_ = p;
            return k;
          }
          inspected[i] = true;
        }
      }
    }

    // Didn't find any reliable action, so we need to guess. Non-frontier cells are preferred.
    for (std::size_t i = 0; i < g_->n_fields(); ++i) {
      const Point p = g_->to_point(i);
      if (g_->opened(p) || g_->flagged(p) || g_->frontier(p)) {
        continue;
      }
      logger_.explored(p, -1);
      g_->OpenWithFrontier(p);
      last_point_ = p;
      return kb_->max_k() + 1;
    }
    for (std::size_t i = 0; i < g_->n_fields(); ++i) {
      const Point p = g_->to_point(i);
      if (g_->opened(p) || g_->flagged(p)) {
        continue;
      }
      logger_.explored(p, -1);
      g_->OpenWithFrontier(p);
      last_point_ = p;
      return kb_->max_k() + 1;
    }

    // That's weird, this case should never occur, because our reasoning should
    // be correct.
    std::cerr << __FILE__ << ":" << __LINE__ << ": Why didn't we choose a point?" << std::endl;
    assert(false);
    return -2;
  }

  Logger& logger() { return logger_; }
  const Logger& logger() const { return logger_; }

 private:
  std::vector<bool> Rectangle(Point p, int radius) {
    const int x0 = p.x;
    const int y0 = p.y;
    std::vector<bool> rectangle(g_->n_fields());
    for (int x = -radius; x <= radius; ++x) {
      set_rectangle(&rectangle, x0 + x, y0 + radius);
      set_rectangle(&rectangle, x0 + x, y0 - radius);
    }
    for (int y = -radius; y <= radius; ++y) {
      set_rectangle(&rectangle, x0 + radius, y0 - y);
      set_rectangle(&rectangle, x0 - radius, y0 - y);
    }
    return rectangle;
  }

  void set_rectangle(std::vector<bool>* rectangle, int x, int y) {
    if (0 <= x && x < static_cast<int>(g_->width()) && 0 <= y && y < static_cast<int>(g_->height())) {
      (*rectangle)[g_->to_index(Point(x, y))] = true;
    }
  }

  Game* g_;
  KnowledgeBase* kb_;
  Logger logger_;
  Point last_point_;
};

#endif  // EXAMPLES_MINESWEEPER_AGENT_H_


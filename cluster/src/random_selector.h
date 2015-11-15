// Adapted from https://gist.github.com/cbsmith/5538174.
#ifndef WC_RANDOM_SELECTOR_H_
#define WC_RANDOM_SELECTOR_H_


#include <random>
#include <iterator>

namespace wc {


class RandomSelector {
public:
  RandomSelector()
    : rng_{std::random_device()()} {}

  template <typename Iter>
  Iter select(Iter begin, Iter end) {
    std::uniform_int_distribution<> dis(0, std::distance(begin, end) - 1);
    std::advance(begin, dis(rng_));
    return begin;
  }

  template <typename Iter>
  Iter operator()(Iter begin, Iter end) {
    return select(begin, end);
  }

  template <typename Container>
  auto operator()(const Container& c) -> decltype(*begin(c))& {
    return *(select(begin(c), end(c)));
  }

private:
  std::default_random_engine rng_;
};


}  // namespace wc

#endif  // WC_RANDOM_SELECTOR_H_

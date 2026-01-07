template <typename T>
void generate_n_unique_rand_in_range(size_t low_bound, size_t high_bound, T *out, size_t n) {
  if (n > high_bound - low_bound + 1)
    throw ::std::invalid_argument("Generate n unique random numbers error - n argument is greater than the range provided by the bounds");

  auto &rng = RandomGenerator::instance();
  ::std::uniform_int_distribution<uint64_t> dist(low_bound, high_bound);
  ::std::unordered_set< size_t > idx_used;

  while (n > 0) {
    size_t rand = dist(rng);

    if (idx_used.contains(rand)) {
      continue;
    }

    idx_used.insert(rand);
    *out++ = rand;
    n--;
  }
}

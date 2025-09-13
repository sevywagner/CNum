template<typename T>
void generate_n_unique_rand_in_range(size_t low_bound, size_t high_bound, T *out, int n) {
  ::std::random_device rd;
  ::std::mt19937 gen(rd());
  ::std::uniform_int_distribution<> dist(low_bound, high_bound);

  ::std::unordered_map< size_t, int > idx_used;

  while (n > 0) {
    size_t rand = dist(gen);

    if (idx_used[rand] > 0) {
      continue;
    }

    idx_used[rand]++;
    
    *out = rand;
    out++;
    n--;
  }
}

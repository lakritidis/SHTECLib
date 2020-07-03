#include "VEPHCEntity.h"

/// Default Constructor
VEPHCEntity::VEPHCEntity() : num_combinations(0), combinations(NULL), predicted_cluster(NULL) { }

/// Destructor
VEPHCEntity::~VEPHCEntity() {
	if (this->combinations) {
		delete [] this->combinations;
	}
}

/// Factorial calculation n!
uint32_t VEPHCEntity::factorial(int32_t x) {
	uint32_t factorial = 1;
	for (int32_t i = 2; i <= x; i++) {
		factorial *= i;
	}
	return factorial;
}

/// n! / (n-k)! calculation
uint32_t VEPHCEntity::factorial_frac(uint32_t n, uint32_t k) {
	uint32_t frac = 1;
	for (uint32_t i = 0; i < k; i++) {
		frac *= (n - i);
	}
	return frac;
}

/// Build the k-Combinations of the tokens of the product title. Create the combination's signature
/// and store each k-Combination in the Combination Lexicon Lc.
void VEPHCEntity::construct_combinations(uint32_t K, class CombinationsLexicon * Lc, class Statistics * stats) {
	uint32_t nc = 0, i = 0, j = 0;
	char combination[1024];

	if (this->num_tokens > 1) {
		for (uint32_t x = 2; x <= K; x++) {
			nc += factorial_frac(this->num_tokens, x) / factorial(x);
		}

		if (nc > 0) {
			this->combinations = new Combination * [nc];
		}
	} else if (this->num_tokens == 1) {
		sprintf(combination, "%d %d", this->tokens[0]->get_id(), this->tokens[0]->get_id());
		this->num_combinations = 1;
		this->combinations = new Combination * [1];
		this->combinations[0] = Lc->insert(combination, 0, 2);
		return;
	}

	uint32_t toks[K], k = 0, l = 0, m = 0, n = 0, o = 0;

	/// Phase 2: Create the combinations/permutations
	/// 2-combinations / 2-permutations
	for (i = 0; i < this->num_tokens; i++) {
		for (j = i + 1; j < this->num_tokens; j++) {
			toks[0] = this->tokens[i]->get_id();
			toks[1] = this->tokens[j]->get_id();

			sprintf(combination, "%d %d", toks[0], toks[1]);

			stats->update_combinations(2);

			this->combinations[this->num_combinations++] = Lc->insert(combination, 0, 2);

//			printf("%d. 2-Combination: %s [%d,%d]\n", this->num_combinations, combination, i, j);
		}
	}

	/// 3-combinations
	if (K >= 3) {
		for (i = 0; i < this->num_tokens; i++) {
			for (j = i + 1; j < this->num_tokens; j++) {
				for (k = j + 1; k < this->num_tokens; k++) {
					toks[0] = this->tokens[i]->get_id();
					toks[1] = this->tokens[j]->get_id();
					toks[2] = this->tokens[k]->get_id();
					qsort(toks, 3, sizeof(uint32_t), &cmp_token_id);

					sprintf(combination, "%d %d %d", toks[0], toks[1], toks[2]);

					stats->update_combinations(3);

					this->combinations[this->num_combinations++] = Lc->insert(combination, 0, 3);

//					printf("%d. 3-Combination: %s [%d,%d,%d - %d,%d,%d]\n",
//						this->num_combinations, combination, i, j, k, toks[0], toks[1], toks[2]);
				}
			}
		}
	}

	/// 4-combinations
	if (K >= 4) {
		for (i = 0; i < this->num_tokens; i++) {
			for (j = i + 1; j < this->num_tokens; j++) {
				for (k = j + 1; k < this->num_tokens; k++) {
					for (l = k + 1; l < this->num_tokens; l++) {

						toks[0] = this->tokens[i]->get_id();
						toks[1] = this->tokens[j]->get_id();
						toks[2] = this->tokens[k]->get_id();
						toks[3] = this->tokens[l]->get_id();
						qsort(toks, 4, sizeof(uint32_t), &cmp_token_id);

						sprintf(combination, "%d %d %d %d", toks[0], toks[1], toks[2], toks[3]);

						stats->update_combinations(4);

						this->combinations[this->num_combinations++] = Lc->insert(combination, 0, 4);

//						printf("%d. 4-Combination: %s [%d,%d,%d,%d - %d,%d,%d,%d]\n",
//							this->num_combinations, combination, i, j, k, l, toks[0], toks[1], toks[2], toks[3]);
					}
				}
			}
		}
	}

	/// 5-combinations
	if (K >= 5) {
		for (i = 0; i < this->num_tokens; i++) {
			for (j = i + 1; j < this->num_tokens; j++) {
				for (k = j + 1; k < this->num_tokens; k++) {
					for (l = k + 1; l < this->num_tokens; l++) {
						for (m = l + 1; m < this->num_tokens; m++) {

							toks[0] = this->tokens[i]->get_id();
							toks[1] = this->tokens[j]->get_id();
							toks[2] = this->tokens[k]->get_id();
							toks[3] = this->tokens[l]->get_id();
							toks[4] = this->tokens[m]->get_id();
							qsort(toks, 5, sizeof(uint32_t), &cmp_token_id);

							sprintf(combination, "%d %d %d %d %d", toks[0], toks[1], toks[2], toks[3],
								toks[4]);

							stats->update_combinations(5);

							this->combinations[this->num_combinations++] = Lc->insert(combination, 0, 5);
//							printf("%d. 5-Combination: %s [%d,%d,%d,%d,%d]\n",
//								this->num_combinations, combination, i, j, k, l, m);
						}
					}
				}
			}
		}
	}

	/// 6-combinations
	if (K >= 6) {
		for (i = 0; i < this->num_tokens; i++) {
			for (j = i + 1; j < this->num_tokens; j++) {
				for (k = j + 1; k < this->num_tokens; k++) {
					for (l = k + 1; l < this->num_tokens; l++) {
						for (m = l + 1; m < this->num_tokens; m++) {
							for (n = m + 1; n < this->num_tokens; n++) {

								toks[0] = this->tokens[i]->get_id();
								toks[1] = this->tokens[j]->get_id();
								toks[2] = this->tokens[k]->get_id();
								toks[3] = this->tokens[l]->get_id();
								toks[4] = this->tokens[m]->get_id();
								toks[5] = this->tokens[n]->get_id();
								qsort(toks, 6, sizeof(uint32_t), &cmp_token_id);

								sprintf(combination, "%d %d %d %d %d %d", toks[0], toks[1], toks[2],
									toks[3], toks[4], toks[5]);

								stats->update_combinations(6);

								this->combinations[this->num_combinations++] = Lc->insert(combination, 0, 6);

//								printf("%d. 6-Combination: %s [%d,%d,%d,%d,%d,%d]\n",
//									this->num_combinations, combination, i, j, k, l, m, n);
							}
						}
					}
				}
			}
		}
	}

	/// 7-combinations
	if (K >= 7) {
		for (i = 0; i < num_tokens; i++) {
			for (j = i + 1; j < num_tokens; j++) {
				for (k = j + 1; k < num_tokens; k++) {
					for (l = k + 1; l < num_tokens; l++) {
						for (m = l + 1; m < num_tokens; m++) {
							for (n = m + 1; n < num_tokens; n++) {
								for (o = n + 1; o < num_tokens; o++) {

									toks[0] = this->tokens[i]->get_id();
									toks[1] = this->tokens[j]->get_id();
									toks[2] = this->tokens[k]->get_id();
									toks[3] = this->tokens[l]->get_id();
									toks[4] = this->tokens[m]->get_id();
									toks[5] = this->tokens[n]->get_id();
									toks[6] = this->tokens[o]->get_id();
									qsort(toks, 7, sizeof(uint32_t), &cmp_token_id);

									sprintf(combination, "%d %d %d %d %d %d %d", toks[0], toks[1],
										toks[2], toks[3], toks[4], toks[5], toks[6]);

									stats->update_combinations(7);

									this->combinations[this->num_combinations++] = Lc->insert(combination, 0, 7);

//									printf("%d. 7-Combination: %s [%d,%d,%d,%d,%d,%d,%d]\n",
//										this->num_combinations, combination, i, j, k, l, m, n, o);
								}
							}
						}
					}
				}
			}
		}
	}
}

/// Compute the scores of all combinations and make the highest-scoring one the cluster of the product
void VEPHCEntity::score_combinations(class Settings * sets, class Statistics * stats, class Token ** tindex) {
	_score_t max_score = 0.0, score = 0.0;
	for (uint32_t i = 0; i < this->num_combinations; i++) {
		score = this->combinations[i]->compute_score_2(sets, stats, tindex);
		if (score >= max_score) {
			max_score = score;
			this->predicted_cluster = this->combinations[i];
		}
	}
}

/// Accessors
inline Combination * VEPHCEntity::get_predicted_cluster() { return this->predicted_cluster; }

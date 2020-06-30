#include "UPMProduct.h"
/// Default Constructor
UPMProduct::UPMProduct() : num_combinations(0), combinations(NULL), predicted_cluster(NULL) {
	this->num_combinations = 0;
	this->combinations = NULL;
	this->predicted_cluster = NULL;
}

/// Destructor
UPMProduct::~UPMProduct() {
	if (this->combinations) {
		delete [] this->combinations;
	}
}

/// Factorial calculation n!
uint32_t UPMProduct::factorial(int32_t x) {
	uint32_t factorial = 1;
	for (int32_t i = 2; i <= x; i++) {
		factorial *= i;
	}
	return factorial;
}

/// n! / (n-k)! calculation
uint32_t UPMProduct::factorial_frac(uint32_t n, uint32_t k) {
	uint32_t frac = 1;
	for (uint32_t i = 0; i < k; i++) {
		frac *= (n - i);
	}
	return frac;
}


/// Identify the semantics of a token and insert it into the token Lexicon.
void UPMProduct::insert_token(uint32_t l, char * t, class TokensLexicon * lx, class TokensLexicon * ulx,
	uint16_t * s, uint16_t * num_mixed) {

		uint16_t token_type = 0, token_sem = 0, ntok = this->num_tokens, i = 0;

		for (i = 0; i < ntok; i++) {
			if (strcmp(this->tokens[i]->get_str(), t) == 0) {
				this->token_weights[i]++;
				return;
			}
		}

		/// If the token is a measurement unit (a search in the units_lexicon is successful), check
		/// the previous token. If the previous is numeric, concatenate the token with the previous
		if (ntok > 0) {
			if (ulx->search(t)) {
				if (this->tokens[ntok - 1]->get_type() == 2) {
					char new_buf[ strlen(this->tokens[ntok - 1]->get_str()) + l + 1 ];
					strcpy(new_buf, this->tokens[ntok - 1]->get_str());
					strcat(new_buf, t);
					new_buf[strlen(this->tokens[ntok - 1]->get_str()) + l] = 0;

					this->tokens[ntok - 1] = lx->insert(new_buf, 3, 2);
					return;
				}
			}
		}

		/// Insert the token into the token Lexicon and store the pointer into this->tokens
		this->tokens[ntok] = lx->insert(t, token_type, token_sem);
		this->token_weights[ntok] = 1.0;

		/// Determine the token type. Check the first character...
		if (t[0] >= 48 && t[0] <= 57) {
			this->tokens[ntok]->set_type(2);
			this->tokens[ntok]->set_sem(4);
		} else {
			this->tokens[ntok]->set_type(1);
			this->tokens[ntok]->set_sem(1);
		}

		/// ... and the rest characters
		for (uint32_t y = 1; y < l; y++) {
			if ((t[y] >= 48 && t[y] <= 57) || t[y] == 44 || t[y] == 46) {
				if (this->tokens[ntok]->get_type() == 1) {
					this->tokens[ntok]->set_type(3);
					if (*num_mixed == 0) {
						this->tokens[ntok]->set_sem(3);
					} else {
						this->tokens[ntok]->set_sem(5);
					}
					(*num_mixed)++;
				}
			} else {
				if (this->tokens[ntok]->get_type() == 2) {
					this->tokens[ntok]->set_type(3);
					if (*num_mixed == 0) {
						this->tokens[ntok]->set_sem(3);
					} else {
						this->tokens[ntok]->set_sem(5);
					}
					(*num_mixed)++;
				}
			}
		}

		/// Check if a mixed token is a possible product attribute. Check if the final characters
		/// of the token correspond to a measurement unit.
		if (this->tokens[ntok]->get_type() == 3) {
			char unit[32];
			uint32_t n = 0, es = 0;
			if (t[0] >= 48 && t[0] <= 57) {
				for (uint32_t y = 1; y < l; y++) {
					if ((t[y] >= 48 && t[y] <= 57) || t[y] == 44 || t[y] == 46) {
						if (n > 0) {
							unit[0] = 0;
							break;
						}
					} else {
						if (n == 0) {
							es = y;
						}
						unit[n++] = t[y];
					}
				}
				unit[n] = 0;

				/// If a measurement unit is identified at the end of the token, check whether the
				/// previous characters are numeric; then declare the token as a product attribute.
				if (ulx->search(unit)) {
					uint32_t is_ms = 1;
					for (uint32_t y = 0; y < es; y++) {
						if ((t[y] >= 48 && t[y] <= 57) || t[y] == 44 || t[y] == 46) {
						} else {
							is_ms = 0;
							break;
						}
					}

					if (is_ms == 1) {
						this->tokens[ntok]->set_sem(2);
					}
				}
			}
		}

		/// In case this is the first token, identify it as a brand name (this always leads to worse results)
		//	if (ntok == 0) { this->tokens[ntok]->sem = 6; }

		this->num_tokens++;

		/// If the available space of the array/s of tokens is exhausted, provide more space.
		if (this->num_tokens >= (*s)) {
			(*s) *= 2;
			this->tokens = (class Token **)realloc(this->tokens, *s * sizeof(class Token *));
			this->token_weights = (_score_t *)realloc(this->token_weights, *s * sizeof(_score_t));
		}
}


/// Tokenize a UPMProduct : Identify the tokens semantics and construct their k-Combinations (UPM-K)
void UPMProduct::tokenize(char * t, uint32_t len, class TokensLexicon * lex, class TokensLexicon * ulex,
	class Statistics * stats) {

		uint16_t alloc_tok = 25, num_mixed = 0;
		uint32_t i = 0, x = 0, y = 0, split = 0;
		char buf[1024], buf2[1024];

		this->tokens = (class Token **)malloc(alloc_tok * sizeof(class Token *));
		this->token_weights = (_score_t *)malloc(alloc_tok * sizeof(_score_t));

		for (i = 0; i < len; i++) {
			if (t[i] == '/' || t[i] == '-') {
				buf[x++] = t[i];
				buf2[y] = 0;
				this->insert_token(y, buf2, lex, ulex, &alloc_tok, &num_mixed);
				y = 0;
				split = 1;

			} else if (t[i] == ' ') {
				buf[x] = 0;
				this->insert_token(x, buf, lex, ulex, &alloc_tok, &num_mixed);

				if (split == 1) {
					buf2[y] = 0;
					this->insert_token(y, buf2, lex, ulex, &alloc_tok, &num_mixed);
					split = 0;
				}
				x = 0;
				y = 0;

			} else {
				/// Case folding
				if (t[i] >= 65 && t[i] <= 90) {
					buf[x++] = t[i] + 32;
					buf2[y++] = t[i] + 32;
				} else {
					buf[x++] = t[i];
					buf2[y++] = t[i];
				}
			}
		}
		buf[x] = 0;
		this->insert_token(x, buf, lex, ulex, &alloc_tok, &num_mixed);

		this->tokens = (class Token **)realloc(this->tokens, this->num_tokens * sizeof(class Token *));
		this->token_weights = (_score_t *)realloc(this->token_weights, this->num_tokens * sizeof(_score_t));

		if (stats) {
			stats->update_titles(this->num_tokens);
		}
/*
		printf("\n========================\nTitle: %s --- Tokens: %d\n\n", t, this->num_tokens);
		for (i = 0; i < this->num_tokens; i++) {
			printf("\t"); this->tokens[i]->display();
		}
		printf("=======================================\n\n");
		getchar();
*/
}

/// Build the k-Combinations of the tokens of the product title. Create the combination's signature
/// and store each k-Combination in the Combination Lexicon Lc.
void UPMProduct::construct_combinations(uint32_t K, class CombinationsLexicon * Lc, class Statistics * stats) {
	uint32_t nc = 0, i = 0, j = 0;
	char combination[1024];
	_score_t distance = 0.0;

	if (this->num_tokens > 1) {
		for (uint32_t x = 2; x <= K; x++) {
			nc += factorial_frac(this->num_tokens, x) / factorial(x);
		}

		if (nc > 0) {
			this->combinations = new Combination * [nc];
		}
	} else if (this->num_tokens == 1) {
		sprintf(combination, "%d %d", this->tokens[0]->get_id(), this->tokens[0]->get_id());
		distance = 0;
		this->num_combinations = 1;
		this->combinations = new Combination * [1];
		this->combinations[0] = Lc->insert(combination, distance, 2);
		return;
	}

	uint32_t toks[K], k = 0, l = 0, m = 0, n = 0, o = 0;

	/// Phase 2: Create the combinations/permutations
	/// 2-combinations / 2-permutations
	for (i = 0; i < this->num_tokens; i++) {
		for (j = i + 1; j < this->num_tokens; j++) {
			toks[0] = this->tokens[i]->get_id();
			toks[1] = this->tokens[j]->get_id();
			qsort(toks, 2, sizeof(uint32_t), &UPMProduct::cmp_token_id);

			sprintf(combination, "%d %d", toks[0], toks[1]);

			stats->update_combinations(2);

			distance = sqrt(pow(i, 2) + pow(j - 1, 2));

			this->combinations[this->num_combinations++] = Lc->insert(combination, distance, 2);

//			printf("%d. 2-Combination: %s [%d,%d] - Dist: %5.3f)\n",
//				this->num_combinations, combination, i, j, distance);
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
					qsort(toks, 3, sizeof(uint32_t), &UPMProduct::cmp_token_id);

					sprintf(combination, "%d %d %d", toks[0], toks[1], toks[2]);

					stats->update_combinations(3);

					distance = sqrt(pow(i, 2) + pow(j - 1, 2) + pow(k - 2, 2));

					this->combinations[this->num_combinations++] = Lc->insert(combination, distance, 3);

//					printf("(%d, %d, %d) - ", this->tokens[i]->get_id(), this->tokens[j]->get_id(),
//						this->tokens[k]->get_id());
//					printf("%d. 3-Combination: %s [%d,%d,%d - %d,%d,%d] - Dist: %5.3f)\n",
//						this->num_combinations, combination, i, j, k, toks[0], toks[1], toks[2], distance);
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
						qsort(toks, 4, sizeof(uint32_t), &UPMProduct::cmp_token_id);

						sprintf(combination, "%d %d %d %d", toks[0], toks[1], toks[2], toks[3]);

						stats->update_combinations(4);

						distance = sqrt(pow(i, 2) + pow(j - 1, 2) + pow(k - 2, 2) + pow(l - 3, 2));

						this->combinations[this->num_combinations++] = Lc->insert(combination, distance, 4);

//						printf("(%d, %d, %d, %d) - ", this->tokens[i]->get_id(), this->tokens[j]->get_id(),
//							this->tokens[k]->get_id(), this->tokens[l]->get_id());
//						printf("%d. 4-Combination: %s [%d, %d, %d, %d] - Dist: %5.3f)\n",
//							this->num_combinations, combination, i, j, k, l, distance);
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
							qsort(toks, 5, sizeof(uint32_t), &UPMProduct::cmp_token_id);

							sprintf(combination, "%d %d %d %d %d", toks[0], toks[1], toks[2], toks[3],
								toks[4]);

							stats->update_combinations(5);

							distance = sqrt(pow(i, 2) + pow(j - 1, 2) + pow(k - 2, 2) + pow(l - 3, 2) +
								pow(m - 4, 2));

							this->combinations[this->num_combinations++] = Lc->insert(combination, distance, 5);

//							printf("%d. 5-Combination: %s [%d,%d,%d,%d,%d] - Dist: %5.3f)\n",
//								this->num_combinations, combination, i, j, k, l, m, distance);
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
								qsort(toks, 6, sizeof(uint32_t), &UPMProduct::cmp_token_id);

								sprintf(combination, "%d %d %d %d %d %d", toks[0], toks[1], toks[2],
									toks[3], toks[4], toks[5]);

								stats->update_combinations(6);

								distance = sqrt(pow(i, 2) + pow(j - 1, 2) + pow(k - 2, 2) + pow(l - 3, 2) +
									pow(m - 4, 2) + pow(n - 5, 2));

								this->combinations[this->num_combinations++] = Lc->insert(combination, distance, 6);

//								printf("%d. 6-Combination: %s [%d,%d,%d,%d,%d,%d] - Dist: %5.3f)\n",
//									this->num_combinations, combination, i, j, k, l, m, n, distance);
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
									qsort(toks, 7, sizeof(uint32_t), &UPMProduct::cmp_token_id);

									sprintf(combination, "%d %d %d %d %d %d %d", toks[0], toks[1],
										toks[2], toks[3], toks[4], toks[5], toks[6]);

									stats->update_combinations(7);

									distance = sqrt(pow(i, 2) + pow(j - 1, 2) + pow(k - 2, 2) +
										pow(l - 3, 2) + pow(m - 4, 2) + pow(n - 5, 2) + pow(o - 6, 2));

									this->combinations[this->num_combinations++] = Lc->insert(combination, distance, 7);

//									printf("%d. 7-Combination: %s [%d,%d,%d,%d,%d,%d,%d] - Dist: %5.3f)\n",
//										this->num_combinations, combination, i, j, k, l, m, n, o, distance);
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
void UPMProduct::score_combinations(class Settings * sets, class Statistics * stats, class Token ** tindex) {
	_score_t max_score = 0.0, score = 0.0;
	for (uint32_t i = 0; i < this->num_combinations; i++) {
		score = this->combinations[i]->compute_score(sets, stats, tindex);
		if (score >= max_score) {
			max_score = score;
			this->predicted_cluster = this->combinations[i];
		}
	}
}

/// Accessors
inline Combination * UPMProduct::get_predicted_cluster() { return this->predicted_cluster; }

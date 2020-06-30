#ifndef UPMPRODUCT_H
#define UPMPRODUCT_H


class UPMProduct : public Product {
	protected:
        uint32_t num_combinations;
        class Combination ** combinations;
        class Combination * predicted_cluster;

	protected:
		uint32_t factorial(int32_t);
		uint32_t factorial_frac(uint32_t, uint32_t);
		void insert_token(uint32_t, char *, class TokensLexicon *,  class TokensLexicon *,
			uint16_t *, uint16_t *);

		static int cmp_token_id(const void * a, const void * b) {
			uint32_t x = *(uint32_t *)a;
			uint32_t y = *(uint32_t *)b;
			return x - y;
		}

	public:
		UPMProduct();
		~UPMProduct();

		void tokenize(char *, uint32_t, class TokensLexicon *, class TokensLexicon *, class Statistics *);
		void construct_combinations(uint32_t, class CombinationsLexicon *, class Statistics *);
		void score_combinations(class Settings *, class Statistics *, class Token **);

		/// Accessors
		class Combination * get_predicted_cluster();
};

#endif // UPMPRODUCT_H

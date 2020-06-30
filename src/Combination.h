#ifndef COMBINATION_H
#define COMBINATION_H


class Combination {
	private:
		char * signature;
		uint16_t num_tokens;

		uint32_t freq;
		_score_t dist_acc;

		class Combination * next;

	public:
		Combination();
		Combination(char *, _score_t, uint16_t);
		~Combination();

		void display(class TokensLexicon *, char *);
		void increase_frequency(uint32_t);
		void increase_dist_acc(_score_t);

		_score_t compute_score(class Settings *, class Statistics *, class Token **);
		_score_t compute_score_2(class Settings *, class Statistics *, class Token **);

		/// Accessors
		char * get_signature();
		uint32_t get_freq();
		uint16_t get_num_tokens();
		_score_t get_dist_acc();
		_score_t get_avg_dist();
		class Combination *get_next();

		/// Mutators
		void set_signature(char *);
		void set_freq(uint32_t);
		void set_dist_acc(_score_t);
		void set_next(class Combination *);
};

#endif // COMBINATION_H

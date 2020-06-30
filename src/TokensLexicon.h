#ifndef TokensLexicon_H
#define TokensLexicon_H

class TokensLexicon {
	private:
		class Token ** hash_table;
		uint32_t mask;
		uint32_t num_slots;  /// The number of slots of the hash table
		uint32_t num_nodes;	 /// The number of elements stored in the hash table.
		uint32_t num_tokens; /// The sum of all frequencies of all tokens (i.e. tokens with duplicates)

	private:
		uint32_t KazLibHash(char *);

	public:
		TokensLexicon();
		TokensLexicon(uint32_t);
		~TokensLexicon();

		void load_units();
		void load_stopwords();
		void display();
		class Token ** reform(uint32_t, class Statistics *);
		void compute_weights(uint32_t);

		class Token * search(char *);
		class Token * insert(char *, uint16_t, uint16_t);
		void erase(char *);
		class Token * get_node(char *);

		uint32_t get_num_nodes();
		uint32_t get_num_tokens();
		uint32_t get_num_slots();
};

#endif // COMBINATION_H

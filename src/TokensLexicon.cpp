#include "TokensLexicon.h"

/// Default Constructor
TokensLexicon::TokensLexicon() :
	hash_table(NULL), mask(0), num_slots(0), num_nodes(0), num_tokens(0) {

}

/// Constructor 2
TokensLexicon::TokensLexicon(uint32_t size) :
	hash_table(NULL), mask(size - 1), num_slots(size), num_nodes(0), num_tokens(0) {

		this->hash_table = new Token * [size];
		for (uint32_t i = 0; i < size; i++) {
			this->hash_table[i] = NULL;
		}
}

/// Destructor
TokensLexicon::~TokensLexicon() {
	class Token * q;
	for (uint32_t i = 0; i < this->num_slots; i++) {
		while (this->hash_table[i] != NULL) {
			q = this->hash_table[i]->get_next();
			delete this->hash_table[i];
			this->hash_table[i] = q;
		}
	}
	delete [] this->hash_table;
}

/// Load some measurement units and their multiples and submultiples into the hash table
void TokensLexicon::load_units() {
	/// Submultiples (micro, pico, nano, milli/mega) & Multiples (kilo, mega, giga, tera)
	const char * smul[] = { "m", "n", "", "k", "g", "t" };
	uint32_t smul_size = (sizeof (smul) / sizeof (const char *));

	const char * u[] = { "b", "hz", "bps", "'", "m", "btu" };
	uint32_t u_size = (sizeof (u) / sizeof (const char *));

	char buf[32];
	uint32_t i = 0, j = 0, l = 0;

	for (i = 0; i < smul_size; i++) {
		for (j = 0; j < u_size; j++) {
			strcpy(buf, smul[i]);
			strcat(buf, u[j]);
			l = strlen(smul[i]) + strlen(u[j]);
			buf[l] = 0;

			this->insert(buf, 0, 0);
		}
	}
}

/// Load the most common English stopwords
void TokensLexicon::load_stopwords() {
	const char * sw[] = {"a", "about", "above", "across", "after", "afterwards", "again",
	"against", "all", "almost", "alone", "along", "already", "also","although","always","am","among",
	"amongst", "amoungst", "amount", "an", "and", "another", "any", "anyhow", "anyone", "anything",
	"anyway", "anywhere", "are", "around", "as", "at", "back", "be", "became", "because", "become",
	"becomes", "becoming", "been", "before", "beforehand", "behind", "being", "below", "beside",
	"besides", "between", "beyond", "bill", "both", "bottom", "but", "by", "call", "can", "cannot",
	"cant", "co", "con", "could", "couldnt", "cry", "de", "describe", "detail", "do", "done", "down",
	"due", "during", "each", "eg", "eight", "either", "eleven", "else", "elsewhere", "empty",
	"enough", "etc", "even", "ever", "every", "everyone", "everything", "everywhere", "except",
	"few", "fifteen", "fify", "fill", "find", "fire", "first", "five", "for", "former", "formerly",
	"forty", "found", "four", "from", "front", "full", "further", "get", "give", "go", "had", "has",
	"hasnt", "have", "he", "hence", "her", "here", "hereafter", "hereby", "herein", "hereupon",
	"hers", "herself", "him", "himself", "his", "how", "however", "hundred", "ie", "if", "in", "inc",
	"indeed", "interest", "into", "is", "it", "its", "itself", "keep", "last", "latter", "latterly",
	"least", "less", "ltd", "made", "many", "may", "me", "meanwhile", "might", "mill", "mine", "more",
	"moreover", "most", "mostly", "move", "much", "must", "my", "myself", "name", "namely", "neither",
	"never", "nevertheless", "next", "nine", "no", "nobody", "none", "noone", "nor", "not", "nothing",
	"now", "nowhere", "of", "off", "often", "on", "once", "one", "only", "onto", "or", "other",
	"others", "otherwise", "our", "ours", "ourselves", "out", "over", "own","part", "per", "perhaps",
	"please", "put", "rather", "re", "same", "see", "seem", "seemed", "seeming", "seems", "serious",
	"several", "she", "should", "show", "side", "since", "sincere", "six", "sixty", "so", "some",
	"somehow", "someone", "something", "sometime", "sometimes", "somewhere", "still", "such",
	"system", "take", "ten", "than", "that", "the", "their", "them", "themselves", "then", "thence",
	"there", "thereafter", "thereby", "therefore", "therein", "thereupon", "these", "they", "thick",
	"thin", "third", "this", "those", "though", "three", "through", "throughout", "thru", "thus",
	"to", "together", "too", "top", "toward", "towards", "twelve", "twenty", "two", "un", "under",
	"until", "up", "upon", "us", "very", "via", "was", "we", "well", "were", "what", "whatever",
	"when", "whence", "whenever", "where", "whereafter", "whereas", "whereby", "wherein", "whereupon",
	"wherever", "whether", "which", "while", "whither", "who", "whoever", "whole", "whom", "whose",
	"why", "will", "with", "within", "without", "would", "yet", "you", "your", "yours", "yourself",
	"yourselves", "the"};

	uint32_t sw_size = (sizeof (sw) / sizeof (const char *));
	for (uint32_t i = 0; i < sw_size; i++) {
		this->insert((char *)sw[i], 0, 0);
	}
}

/// Search the hash table for a given token. If exists, increase its frequency by 1 and return
/// the pointer to the object. Return NULL otherwise.
class Token * TokensLexicon::search(char *t) {
	uint32_t HashValue = KazLibHash(t) & this->mask;
	if (this->hash_table[HashValue] != NULL) {
		class Token *q;

		/// Traverse the linked list that represents the chain.
		for (q = this->hash_table[HashValue]; q != NULL; q = q->get_next()) {
			if (strcmp(q->get_str(), t) == 0) {
				this->num_tokens++;
				q->set_freq( q->get_freq() + 1 );

				return q; /// Return the pointer and exit
			}
		}
	}

	return NULL; /// Entry not found, return NULL
}

/// Search the hash table for a given token. If exists, return its frequency.
inline class Token * TokensLexicon::get_node(char * t) {
	uint32_t HashValue = KazLibHash(t) & this->mask;
	if (this->hash_table[HashValue] != NULL) {
		class Token * q;

		/// Traverse the linked list that represents the chain.
		for (q = this->hash_table[HashValue]; q != NULL; q = q->get_next()) {
			if (strcmp(q->get_str(), t) == 0) {
				return q; /// Return the pointer and exit
			}
		}
	}

	return NULL; /// Literal not found, return 0
}

/// Insert an element into the hash table
class Token * TokensLexicon::insert(char * t, uint16_t type, uint16_t sem) {
	class Token * res;

	/// In case the element exists in the hash table, increase its frequency and return
	if ((res = this->search(t)) != NULL) {
		return res;
	}

	/// In case the element does not exist in the hash table, insert it.
	/// Create a new record and re-assign the linked list's head.
	uint32_t HashValue = KazLibHash(t) & this->mask;

	class Token * record = new Token(t, ++this->num_nodes, type, sem);
	record->set_next(this->hash_table[HashValue]);
	this->hash_table[HashValue] = record;

	this->num_tokens++;
	return record;
}

/// Erase an element from the hash table
void TokensLexicon::erase(char * t) {

	uint32_t HashValue = KazLibHash(t) & this->mask;

	class Token * q = this->hash_table[HashValue], * p = NULL;

	while (q != NULL) {
		if (strcmp(q->get_str(), t) == 0) {
			q->set_freq( q->get_freq() - 1 );
			this->num_tokens--;

			if (q->get_freq() == 0) {
				if (!p) {
					this->hash_table[HashValue] = q->get_next();
				} else {
					p->set_next( q->get_next() );
				}

				delete q;
				this->num_nodes--;
			}

			return;
		}

		p = q;
		q = q->get_next();
	}
}

/// Display the Hash table elements to stdout
void TokensLexicon::display() {
	class Token * q;

	/// Iterate down the Hash Table and display non NULL keys.
	for (uint32_t i = 0; i < this->num_slots; i++) {
		if (this->hash_table[i] != NULL) {
			printf("Slot %d\n", i);
			for (q = this->hash_table[i]; q != NULL; q = q->get_next()) {
				printf("\t"); q->display();
			}
		}
	}
}

/// Move the Hash table into a standard table
class Token ** TokensLexicon::reform(uint32_t N, class Statistics * stats) {
	class Token * q;
	class Token ** thtn = new Token * [ this->num_nodes + 1 ];

	stats->set_num_tokens(this->num_nodes);

	/// Iterate down the Hash Table and display non NULL keys.
	for (uint32_t i = 0; i < this->num_slots; i++) {
		if (this->hash_table[i] != NULL) {
			for (q = this->hash_table[i]; q != NULL; q = q->get_next()) {
				q->compute_weight(N);
				stats->update_zone_tokens( q->get_sem(), q->get_freq() );
				thtn[ q->get_id() ] = q;
			}
		}
	}

	return thtn;
}

/// Traverse the Hash table and compute the IDFs for all tokens
void TokensLexicon::compute_weights(uint32_t N) {
	class Token * q;

	/// Iterate down the Hash Table and display non NULL keys.
	for (uint32_t i = 0; i < this->num_slots; i++) {
		if (this->hash_table[i] != NULL) {
			for (q = this->hash_table[i]; q != NULL; q = q->get_next()) {
				q->compute_weight(N);
//				q->display(); getchar();
			}
		}
	}
}


/// The Hash Function
uint32_t TokensLexicon::KazLibHash (char *key) {
   static unsigned long randbox[] = {
       0x49848f1bU, 0xe6255dbaU, 0x36da5bdcU, 0x47bf94e9U,
       0x8cbcce22U, 0x559fc06aU, 0xd268f536U, 0xe10af79aU,
       0xc1af4d69U, 0x1d2917b5U, 0xec4c304dU, 0x9ee5016cU,
       0x69232f74U, 0xfead7bb3U, 0xe9089ab6U, 0xf012f6aeU,
	};

	char *str = key;
	uint32_t acc = 0;

	while (*str) {
		acc ^= randbox[(*str + acc) & 0xf];
		acc = (acc << 1) | (acc >> 31);
		acc &= 0xffffffffU;
		acc ^= randbox[((*str++ >> 4) + acc) & 0xf];
		acc = (acc << 2) | (acc >> 30);
		acc &= 0xffffffffU;
	}
	return acc;
}

/// Accessors
inline uint32_t TokensLexicon::get_num_nodes() { return this->num_nodes; }
inline uint32_t TokensLexicon::get_num_slots() { return this->num_slots; }
inline uint32_t TokensLexicon::get_num_tokens() { return this->num_tokens; }

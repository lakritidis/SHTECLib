#include "VEPHC.h"

/// VEPHC class Constructor: This calls the constructor of ClusteringAlgorithm<T,U> which in turn
/// calls the constructor of ClusteringMethod<T>
VEPHC::VEPHC(class Settings * p) :
	ClusteringAlgorithm<VEPHCEntity, VEPHCEntity>(p),
	tokens_index(NULL),
	combinations_lexicon(new CombinationsLexicon(16 * 1048576)),
	K(p->get_upm_k()),
	alpha(p->get_upm_a()),
	beta(p->get_upm_b()) {

}

/// Destructor
VEPHC::~VEPHC() {
	if (this->combinations_lexicon) {
		delete this->combinations_lexicon;
	}

	if (this->tokens_index) {
		delete [] this->tokens_index;
	}
}

/// Execute VEPHC
void VEPHC::exec() {
	uint32_t i = 0, num_slots = 1048576;

	this->initialize();

#ifdef __linux__
	struct timespec ts, te;
	clock_gettime(CLOCK_REALTIME, &ts);
	double duration = 0.0;
#endif

	/// Convert TokensLexicon to a perfect hash table for constant O(1) time searches by tokenID.
	this->tokens_index = this->lex->reform(this->num_entities, this->stats);

	/// Create the combinations from each product title
	printf("Computing k-Combinations...\n"); fflush(NULL);
	for (i = 0; i < this->num_entities; i++) {
		this->e[i]->construct_combinations(this->K, this->combinations_lexicon, this->stats);
		this->e[i]->sort_feature_vector();
	}

	/// Compute the scores of the combinations; identify the dominating combination and declare it
	/// as the cluster of the product.
	printf("Running '%s' on '%s'...\n", this->PARAMS->get_algorithm_name(), this->PARAMS->get_dataset());
	fflush(NULL);

	this->cluster_universe = new ClusterUniverse<VEPHCEntity >(num_slots);

	this->stats->compute_final_values();

	for (i = 0; i < this->num_entities; i++) {
		this->e[i]->score_combinations(this->PARAMS, this->stats, this->tokens_index);
		this->cluster_universe->insert_node(this->e[i]);
	}

	this->num_clusters = this->cluster_universe->get_num_nodes();
	this->num_alloc_clusters = this->num_clusters;

	if (this->PARAMS->get_perform_verification()) {
		class Cluster<VEPHCEntity> ** temp_clusters = this->cluster_universe->convert_to_array();
		this->HC_Stage(temp_clusters, this->PARAMS->get_h_threshold(), this->PARAMS->get_c_threshold());
	} else {
		this->clusters = this->cluster_universe->convert_to_array();
	}

#ifdef __linux__
		clock_gettime(CLOCK_REALTIME, &te);
		duration = (te.tv_sec - ts.tv_sec) + (double)(te.tv_nsec - ts.tv_nsec) / (double)(1000000000);
		this->evaluators[0].set_duration(duration);
		printf("\t -> %10.3f sec\n", duration); fflush(NULL);
#else
		this->evaluators[0].set_duration(duration);
		printf("\n"); fflush(NULL);
#endif

	/// Store the pairwise matches to the appropriate file
	this->evaluators[0].set_num_algo_clusters(this->num_clusters);
	this->evaluators[0].set_nmi( this->evaluate_nmi(0) );
	uint32_t num_matches = 0;

	for (i = 0; i < this->num_clusters; i++) {
		num_matches += this->clusters[i]->create_pairwise_matches( this->output_files[0] );
		delete this->clusters[i];
	}

	this->evaluators[0].set_num_algo_matches(num_matches);
	free(this->clusters);

	delete this->cluster_universe;

	this->finalize();
}


/// The Hash Function
uint32_t VEPHC::KazLibHash (char *key) {
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

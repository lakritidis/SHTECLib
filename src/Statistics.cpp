#include "Statistics.h"
/// Default Constructor
Statistics::Statistics() :
	duration(0.0), num_combinations(0), combinations_length_acc(0), avg_combinations_length(0.0),
	num_titles(0), max_title_length(0), min_title_length(100000), title_length_acc(0), avg_title_length(0.0),
	num_vendors(0), num_tokens(0), num_clusters(0), num_corr_clusters(0), max_cluster_size(0), min_cluster_size(100000),
	cluster_size_acc(0), avg_cluster_size(0.0), zone_token_acc{0}, zone_product_acc{0} {

#ifdef __linux__
		clock_gettime(CLOCK_REALTIME, &this->ts);
#elif _WIN32

#endif
}

/// Destructor
Statistics::~Statistics() { }

/// Compute the elapsed time from the initialization of the structure
void Statistics::get_elapsed_time() {
#ifdef __linux__
		struct timespec te;
		clock_gettime(CLOCK_REALTIME, &te);
		this->duration = (te.tv_sec - this->ts.tv_sec) +
			(double)(te.tv_nsec - this->ts.tv_nsec) / (double)(1000000000);
#elif _WIN32
		printf("Cannot compute duration");
#endif
}

/// Mutators
inline void Statistics::update_combinations(uint32_t len) {
	this->combinations_length_acc += len;
	this->num_combinations++;
}

inline void Statistics::update_titles(uint32_t len) {
	this->num_titles++;
	this->title_length_acc += len;
	if (len > this->max_title_length) { this->max_title_length = len; }
	if (len <= this->min_title_length) { this->min_title_length = len; }
}

inline void Statistics::compute_final_values() {
	this->avg_combinations_length = (double)this->combinations_length_acc / (double)this->num_combinations;
	this->avg_title_length = (double)this->title_length_acc / (double)this->num_titles;
}

inline void Statistics::update_clusters(uint32_t len) {
	if (len > this->max_cluster_size) { this->max_cluster_size = len; }
	if (len <= this->min_cluster_size) { this->min_cluster_size = len; }
}

inline void Statistics::increase_clusters() {
	this->num_clusters++;
}

inline void Statistics::update_zone_tokens(uint32_t v, uint32_t f) {
	this->zone_token_acc[v] += f;
}

inline void Statistics::update_zone_products(uint32_t v) {
	this->zone_product_acc[v]++;
}

inline void Statistics::set_num_vendors(uint32_t v) {
	this->num_vendors = v;
}

inline void Statistics::set_num_tokens(uint32_t v) {
	this->num_tokens = v;
}

inline void Statistics::set_num_clusters(uint32_t v) {
	this->num_clusters = v;
}

inline void Statistics::set_num_corr_clusters(uint32_t v) {
	this->num_corr_clusters = v;
}

/// Accessors
inline uint32_t Statistics::get_num_combinations() { return this->num_combinations; }
inline _score_t Statistics::get_avg_combinations_length() { return this->avg_combinations_length; }
inline uint32_t Statistics::get_num_titles() { return this->num_titles; }
inline uint32_t Statistics::get_max_title_length() { return this->max_title_length; }
inline uint32_t Statistics::get_min_title_length() { return this->min_title_length; }
inline _score_t Statistics::get_avg_title_length() { return this->avg_title_length; }
inline uint32_t Statistics::get_num_clusters() { return this->num_clusters; }
inline uint32_t Statistics::get_num_corr_clusters() { return this->num_corr_clusters; }
inline uint32_t Statistics::get_num_vendors() { return this->num_vendors; }
inline uint32_t Statistics::get_num_tokens() { return this->num_tokens; }
inline uint32_t Statistics::get_num_zone_tokens(uint32_t v) { return this->zone_token_acc[v]; }
inline uint32_t Statistics::get_max_cluster_size() { return this->max_cluster_size; }
inline uint32_t Statistics::get_min_cluster_size() { return this->min_cluster_size; }
inline _score_t Statistics::get_avg_cluster_size() { return this->avg_cluster_size; }
inline _score_t Statistics::get_avg_zone_tokens(uint32_t v) { return (_score_t)this->zone_token_acc[v] / this->num_titles; }
inline _score_t Statistics::get_avg_zone_products(uint32_t v) { return (_score_t)this->zone_product_acc[v] / this->num_titles; }

/// Display the structure's contents
void Statistics::display() {
	this->avg_combinations_length = (_score_t)this->combinations_length_acc / this->num_combinations;
	this->avg_title_length = (_score_t)this->title_length_acc / this->num_titles;

	printf("\n\n==== Statistics ==============================================\n");
	printf("Total Duration: %5.3f sec\n", this->duration);
	printf("-------------------------------------------------------------\n");
	printf("Computed Combinations: %d\n", this->num_combinations);
	printf("Average Combinations Length: %5.3f\n", this->avg_combinations_length);
	printf("-------------------------------------------------------------\n");
	printf("Total Titles: %d\n", this->num_titles);
	printf("Total Products: %d\n", this->num_clusters);
	printf("Total Vendors: %d\n", this->num_vendors);
	printf("Average Title Length: %5.3f\n", this->avg_title_length);
	printf("Max Title Length: %d\n", this->max_title_length);
	printf("Min Title Length: %d\n", this->min_title_length);
	printf("-------------------------------------------------------------\n");
	printf("Num clusters: %d\n", this->num_clusters);
	printf("Max cluster size: %d\n", this->max_cluster_size);
	printf("Min cluster size: %d\n", this->min_cluster_size);
	printf("-------------------------------------------------------------\n");
	printf("Token Semantics Analysis\n");
	printf("Token Type   |");
	for (uint32_t i = 0; i < NUM_ZONES; i++) { printf(" %d\t|", i); }
	printf("\nNum Products |");
	for (uint32_t i = 0; i < NUM_ZONES; i++) { printf(" %d\t|", this->zone_product_acc[i]); }
	printf("\nNum Tokens   |");
	for (uint32_t i = 0; i < NUM_ZONES; i++) { printf(" %d\t|", this->zone_token_acc[i]); }
	printf("\n-------------------------------------------------------------\n");
}

#ifndef SETTINGS_H
#define SETTINGS_H

#include <vector>

class Settings {
	private:
		/// Universal Settings
		char algorithm_name[32];
		uint16_t distance_metric;
		uint16_t algorithm;
		char dataset[256];
		char dataset_path[2048];
		char results_path[2048];
		uint32_t max_entities;

		/// Universal Settings for Clustering
		_score_t similarity_threshold;
		_score_t min_distance;
		uint16_t num_zones;
		uint16_t low_threshold;
		uint16_t high_threshold;
		uint16_t entities_type;

		/// Algorithm-specific hyper-parameters
		uint16_t upm_k;
		_score_t upm_a;
		_score_t upm_b;
		_score_t gsdmm_a;
		_score_t gsdmm_b;
		uint32_t clusters;
		uint32_t minpoints;
		uint16_t iterations;
		uint16_t linkage;
		uint16_t kmeans_type;

		bool perform_verification;
		bool spectral_autotune;
		bool verbose;

		std::vector<std::string> supported_algorithms;
		std::vector<std::string> supported_distances;

	public:
		Settings();
		~Settings();

		void read_from_input(int, char **);

		/// Mutators
		void set_algorithm(uint16_t);
		void set_algorithm_name(char *);
		void set_distance_metric(uint16_t);
		void set_dataset(char *);
		void set_dataset_path(char *);
		void set_results_path(char *);
		void set_max_entities(uint32_t);
		void set_similarity_threshold(_score_t);
		void set_min_distance(_score_t);
		void set_num_zones(uint16_t);
		void set_low_threshold(uint16_t);
		void set_high_threshold(uint16_t);
		void set_entities_type(uint16_t);
		void set_upm_k(uint16_t);
		void set_upm_a(_score_t);
		void set_upm_b(_score_t);
		void set_gsdmm_a(_score_t);
		void set_gsdmm_b(_score_t);
		void set_clusters(uint32_t);
		void set_minpoints(uint32_t);
		void set_iterations(uint16_t);
		void set_linkage(uint16_t);
		void set_perform_verification(bool);
		void set_kmeans_type(uint16_t);
		void set_spectral_autotune(bool);
		void set_verbose(bool);

		/// Accessors
		uint16_t get_algorithm();
		char * get_algorithm_name();
		uint16_t get_distance_metric();
		char * get_dataset();
		char * get_dataset_path();
		char * get_results_path();
		uint32_t get_max_entities();
		_score_t get_similarity_threshold();
		_score_t get_min_distance();
		uint16_t get_num_zones();
		uint16_t get_low_threshold();
		uint16_t get_high_threshold();
		uint16_t get_entities_type();
		uint16_t get_upm_k();
		_score_t get_upm_a();
		_score_t get_upm_b();
		_score_t get_gsdmm_a();
		_score_t get_gsdmm_b();
		uint32_t get_clusters();
		uint32_t get_minpoints();
		uint16_t get_iterations();
		uint16_t get_linkage();
		bool get_perform_verification();
		uint16_t get_kmeans_type();
		bool get_spectral_autotune();
		bool get_verbose();
};

#endif // SETTINGS_H

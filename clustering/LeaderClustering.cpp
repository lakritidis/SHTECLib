#include "LeaderClustering.h"

/// Leader Clustering class Constructor: This calls the constructor of ClusteringAlgorithm<T,U>
/// which in turn calls the constructor of ClusteringMethod<T>
template <class T, class U> LeaderClustering<T,U>::LeaderClustering(class Settings * p) : ClusteringAlgorithm<T,U>(p) {

}

/// Leader Clustering Destructor
template <class T, class U>LeaderClustering<T,U>::~LeaderClustering() {

}

/// ///////////////////////////////////////////////////////////////////////////////////////////////
/// Leader Clustering Algorithm
template <class T, class U> void LeaderClustering<T,U>::exec() {
	uint32_t i = 0, j = 0, k = 0, cand = 0;
	_score_t min_distance = INIT_DISTANCE, simt = 0.0, dis = 0.0;
	uint32_t low_t = this->PARAMS->get_low_threshold();
	uint32_t high_t = this->PARAMS->get_high_threshold();

	this->initialize();

	printf("Running '%s' on '%s'...\n", this->PARAMS->get_algorithm_name(), this->PARAMS->get_dataset());
	fflush(NULL);

#ifdef __linux__
	struct timespec ts, te;
	double duration = 0.0, t_duration = 0.0;
#endif

	this->num_alloc_clusters = 1000;

	/// For 10 similarity thresholds
	for (k = low_t; k < high_t; k++) {

#ifdef __linux__
		clock_gettime(CLOCK_REALTIME, &ts);
#endif

		simt = (_score_t)(k + 1) / 10;
		printf("\tSimilarity Threshold :%5.1f", simt); fflush(NULL);

		this->num_clusters = 0;
		cand = 0;
		min_distance = INIT_DISTANCE;

		this->clusters = (class Cluster<U> **)malloc(this->num_alloc_clusters * sizeof(class Cluster<U> *));

		/// Execute the algorithm
		for (i = 0; i < this->num_entities; i++) {
			cand = 0;
			min_distance = INIT_DISTANCE;

			/// Find the most similar cluster
			for (j = 0; j < this->num_clusters; j++) {
				dis = 1.0 - this->e[i]->cosine_similarity(this->clusters[j]->get_representative());

				if (dis < min_distance) {
					min_distance = dis;
					cand = j;
				}
			}

			/// If the maximum similarity exceeds the threshold, insert the entity into the cluster
			if (min_distance < simt) {
				this->clusters[cand]->insert_entity( this->e[i] );
//				this->clusters[cand]->compute_clustroid();

			/// otherwise, create a new cluster and insert the entity into the new cluster
			} else {
				this->clusters[this->num_clusters] = new class Cluster<U>(this->num_clusters);
				this->clusters[this->num_clusters]->insert_entity(this->e[i]);
				this->clusters[this->num_clusters]->set_representative(this->e[i]);
				this->num_clusters++;

				if (this->num_clusters >= this->num_alloc_clusters) {
					this->num_alloc_clusters *= 2;
					this->clusters = (class Cluster<U> **)realloc
							(this->clusters, this->num_alloc_clusters * sizeof(class Cluster<U> *));
				}
			}
		}

		if (this->verification) {
			this->perform_verification(2);
		}

#ifdef __linux__
		clock_gettime(CLOCK_REALTIME, &te);
		duration = (te.tv_sec - ts.tv_sec) + (double)(te.tv_nsec - ts.tv_nsec) / (double)(1000000000);
		this->evaluators[k].set_duration(duration);
		t_duration += duration;
		printf("\t -> %10.3f sec (%10.3f sec)\n", duration, t_duration); fflush(NULL);
#else
		this->evaluators[k].set_duration(0);
		printf("\n"); fflush(NULL);
#endif

		/// Store the pairwise matches to the appropriate file
		this->evaluators[k].set_num_algo_clusters(this->num_clusters);
		this->evaluators[k].set_nmi( this->evaluate_nmi(k) );
		uint32_t num_matches = 0;

		for (i = 0; i < this->num_clusters; i++) {
			num_matches += this->clusters[i]->create_pairwise_matches( this->output_files[k] );
			delete this->clusters[i];
		}

		this->evaluators[k].set_num_algo_matches(num_matches);
		free(this->clusters);
	}

	this->finalize();
}

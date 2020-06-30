#include "DBSCAN.h"

/// DBSCAN class Constructor: This calls the constructor of ClusteringAlgorithm<T,U> which in turn
/// calls the constructor of ClusteringMethod<T>
template <class T, class U> DBSCAN<T,U>::DBSCAN(class Settings * p) :
	ClusteringAlgorithm<T,U>(p), min_points(p->get_minpoints()) {
}

/// DBSCAN Destructor
template <class T, class U> DBSCAN<T,U>::~DBSCAN() {

}

/// DBSCAN Assistant function: It Finds the neighbors of the given entity q by computing the Cos-Sim with all entities
template <class T, class U> uint32_t DBSCAN<T,U>::dbscan_range_query(T *** n, uint32_t * num_an, T *q , _score_t t) {
	uint32_t i = 0, num_neighbors = 0;
	_score_t sim = 0.0;
	T * e;

	for (i = 0; i < this->num_entities; i++) {
		e = this->e[i];

		sim = q->cosine_similarity(e);

		if (sim > t) {
			(*n)[num_neighbors] = e;
			num_neighbors++;

			if (num_neighbors >= (*num_an)) {
				(*num_an) *= 2;
				*n = (T **)realloc(*n, *num_an * sizeof(T *));
			}
		}

	}
	return num_neighbors;
}

/// ///////////////////////////////////////////////////////////////////////////////////////////////
/// DBSCAN Clustering Algorithm
template <class T, class U> void DBSCAN<T,U>::exec() {
	uint32_t i = 0, j = 0, k = 0, l = 0, cand = 0;
	uint32_t low_t = this->PARAMS->get_low_threshold();
	uint32_t high_t = this->PARAMS->get_high_threshold();
	uint32_t n_nbors = 0, n_new_nbors = 0;
	uint32_t n_alloc_nbors = 1000, n_alloc_new_nbors = 1000;
	_score_t simt = 0.0;
	T * e;

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
		printf("\tSimilarity Threshold : %5.1f", simt); fflush(NULL);

		for (i = 0; i < this->num_entities; i++) {
			this->e[i]->set_type(0);
		}

		this->num_clusters = 0;

		this->clusters = (class Cluster<U> **)malloc (this->num_alloc_clusters * sizeof(class Cluster<U> *));

		T ** nbors = (T **)malloc(n_alloc_nbors * sizeof(T *));
		T ** new_nbors = (T **)malloc(n_alloc_new_nbors * sizeof(T *));

		/// For each entity
		for (i = 0; i < this->num_entities; i++) {
			e = this->e[i];
			n_nbors = 0;
//			printf("Checking Product %d with similarity %2.1f, type=%d.\n", i, simt, e->get_type());
//			e->display();

			/// If it has not been previously processed in inner loop
			if (e->get_type() == 0) {

				n_nbors = this->dbscan_range_query(&nbors, &n_alloc_nbors, e, simt);

				/// Make this point a noise (outlier)
				if (n_nbors < this->min_points) {
					e->set_type(2);

				/// otherwise continue
				} else {
					/// Create a new cluster and put e there
					e->set_type(1);
					this->clusters[this->num_clusters] = new class Cluster<U>(this->num_clusters);
					this->clusters[this->num_clusters]->set_representative(e);
					this->clusters[this->num_clusters]->insert_entity(e);
					this->num_clusters++;

					if (this->num_clusters >= this->num_alloc_clusters) {
						this->num_alloc_clusters *= 2;
						this->clusters = (class Cluster<U> **)realloc(this->clusters,
								this->num_alloc_clusters * sizeof(class Cluster<U> **));
					}

					/// Expand neighbors
					for (j = 0; j < n_nbors; j++) {
//						printf("\t%d. ", j); nbors[j]->display();
						if (nbors[j]->get_type() == 0 || nbors[j]->get_type() == 2) {

							/// Insert the entity into the cluster
							cand = this->num_clusters - 1;
							nbors[j]->set_type(1);
							this->clusters[cand]->insert_entity(nbors[j]);

							n_new_nbors = this->dbscan_range_query
									(&new_nbors, &n_alloc_new_nbors, nbors[j], simt);

							if (n_new_nbors >= this->min_points) {
								for (l = 0; l < n_new_nbors; l++) {
									nbors[n_nbors++] = new_nbors[l];

									if (n_nbors >= n_alloc_nbors) {
										n_alloc_nbors *= 2;
										nbors = (T **)realloc(nbors, n_alloc_nbors * sizeof(T *));
									}
								}
							}
//							printf("Now there are %d neighbors, CONTINUE\n", n_nbors);
						}
					}
				}
			}
		}
//		printf("==== Num clusters: %d\n", num_clusters);

		/// Run the verification stage (products only)
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

		free(nbors);
		free(new_nbors);
	}

	this->finalize();
}

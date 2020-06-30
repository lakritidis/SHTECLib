#include "ManualClustering.h"

/// Manual Clustering class Constructor: This calls the constructor of ClusteringAlgorithm<T,U>
/// which in turn calls the constructor of ClusteringMethod<T>
template <class T, class U> ManualClustering<T,U>::ManualClustering(class Settings * p) : ClusteringAlgorithm<T,U>(p) {

}

/// Leader Clustering Destructor
template <class T, class U>ManualClustering<T,U>::~ManualClustering() {

}

/// ///////////////////////////////////////////////////////////////////////////////////////////////
/// Leader Clustering Algorithm
template <class T, class U> void ManualClustering<T,U>::exec() {
	uint32_t i = 0;
	printf("Running '%s' on '%s'...\n", this->PARAMS->get_algorithm_name(), this->PARAMS->get_dataset());
	fflush(NULL);

#ifdef __linux__
	struct timespec ts, te;
	clock_gettime(CLOCK_REALTIME, &ts);
	double duration = 0.0;
#endif

	this->num_alloc_clusters = 2;
	this->num_clusters = 2;

	this->clusters = (class Cluster<U> **)malloc(this->num_alloc_clusters * sizeof(class Cluster<U> *));
	this->clusters[0] = new Cluster<U>(1);
	this->clusters[1] = new Cluster<U>(2);

/*
	/// Example 1
	this->clusters[0]->insert_entity( this->e[0] );
	this->clusters[0]->insert_entity( this->e[1] );
	this->clusters[0]->insert_entity( this->e[2] );
	this->clusters[0]->insert_entity( this->e[5] );
	this->clusters[0]->insert_entity( this->e[6] );
	this->clusters[0]->insert_entity( this->e[7] );
	this->clusters[0]->insert_entity( this->e[8] );
	this->clusters[0]->insert_entity( this->e[10] );
	this->clusters[0]->insert_entity( this->e[11] );
	this->clusters[0]->insert_entity( this->e[12] );

	this->clusters[1]->insert_entity( this->e[3] );
	this->clusters[1]->insert_entity( this->e[4] );
	this->clusters[1]->insert_entity( this->e[9] );
	this->clusters[1]->insert_entity( this->e[13] );
	this->clusters[1]->insert_entity( this->e[14] );
	this->clusters[1]->insert_entity( this->e[15] );
	this->clusters[1]->insert_entity( this->e[16] );
	this->clusters[1]->insert_entity( this->e[17] );
	this->clusters[1]->insert_entity( this->e[18] );
	this->clusters[1]->insert_entity( this->e[19] );
*/

	/// Example 2
	this->clusters[0]->insert_entity( this->e[0] );
	this->clusters[0]->insert_entity( this->e[1] );
	this->clusters[0]->insert_entity( this->e[2] );
	this->clusters[0]->insert_entity( this->e[10] );
	this->clusters[0]->insert_entity( this->e[11] );
	this->clusters[0]->insert_entity( this->e[12] );
	this->clusters[0]->insert_entity( this->e[13] );
	this->clusters[0]->insert_entity( this->e[14] );
	this->clusters[0]->insert_entity( this->e[15] );
	this->clusters[0]->insert_entity( this->e[16] );

	this->clusters[1]->insert_entity( this->e[3] );
	this->clusters[1]->insert_entity( this->e[4] );
	this->clusters[1]->insert_entity( this->e[5] );
	this->clusters[1]->insert_entity( this->e[6] );
	this->clusters[1]->insert_entity( this->e[7] );
	this->clusters[1]->insert_entity( this->e[8] );
	this->clusters[1]->insert_entity( this->e[9] );
	this->clusters[1]->insert_entity( this->e[17] );
	this->clusters[1]->insert_entity( this->e[18] );
	this->clusters[1]->insert_entity( this->e[19] );

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

	this->finalize();
}

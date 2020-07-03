/// ///////////////////////////////////////////////////////////////////////////////////////////////
/// STCLib : Short Text Clustering Library
///
/// STCLib  is a small collection of C++  implementations of the most popular  algorithms for  text
/// clustering. This software can be compiled as a command  line tool by modifying the value of the
/// COOMAND_LINE declarative in line 10. Otherwise, the various parameters of the algorithms can be
/// directly determined in Settings.cpp.
///
/// L. Akritidis, May, 2020
/// ///////////////////////////////////////////////////////////////////////////////////////////////

#define COMMAND_LINE false

#include "stdio.h"
#include "stdlib.h"
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <algorithm>
#include <iostream>

#define SIMILARITY_THRESHOLD 0.5
#define NUM_ZONES 6

#define INIT_DISTANCE 1000000.0

typedef float _score_t;

/// Libraries
#include "lib/tclap/CmdLine.h"
#include "src/Settings.cpp"

/// Generic Clustering Classes
#include "src/Statistics.cpp"
#include "src/EvaluationMeasures.cpp"
#include "src/Token.cpp"
#include "src/TokensLexicon.cpp"
#include "src/Feature.cpp"
#include "src/Entity.cpp"
#include "src/Product.cpp"
#include "src/EntitiesGroup.cpp"
#include "src/Cluster.cpp"
#include "src/ClusterHashNode.cpp"
#include "src/EntityCluster.cpp"

/// UPM/VEPHC classes
#include "src/Combination.cpp"
#include "src/CombinationsLexicon.cpp"
#include "src/UPMProduct.cpp"
#include "src/VEPHCEntity.cpp"
#include "src/ClusterUniverse.cpp"

/// Algorithms Classes
#include "clustering/ClusteringMethod.cpp"
#include "clustering/SimilarityMeasure.cpp"
#include "clustering/EditSimilarity.cpp"
#include "clustering/ClusteringAlgorithm.cpp"

/// Algorithms Implementations
#include "clustering/ManualClustering.cpp"
#include "clustering/KMeans.cpp"
#include "clustering/Agglomerative.cpp"
#include "clustering/LeaderClustering.cpp"
#include "clustering/DBSCAN.cpp"
//#include "clustering/SpectralClustering.cpp"
#include "clustering/VEPHC.cpp"
#include "clustering/UPM.cpp"
#include "clustering/GSDMM.cpp"

int main(int argc, char ** argv) {
	class Settings * params = new Settings();
//	params->read_from_input(argc, argv);

	char filepath[4096];
	sprintf(filepath, "%s%s", params->get_dataset_path(), params->get_dataset());

	/// ////////////////////////////////////////////////////////////////////////////////////////////
	/// Entities Clustering
	if (params->get_entities_type() == 1) {
		/// Token-Based String Similarity Metrics
		if (params->get_algorithm() == 1 || params->get_algorithm() == 2 || params->get_algorithm() == 3 ||
			params->get_algorithm() == 5 || params->get_algorithm() == 6 || params->get_algorithm() == 7) {

			class SimilarityMeasure<Entity> * calgo = new SimilarityMeasure<Entity>(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate_f1();
			delete calgo;
		}

		/// Edit Distance (Similarity)
		if (params->get_algorithm() == 4) {
			class EditSimilarity<Entity> * calgo = new EditSimilarity<Entity>(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate_f1();
			delete calgo;
		}

		/// Leader Clustering
		if (params->get_algorithm() == 8) {
			class LeaderClustering<Entity, Entity> * calgo = new LeaderClustering<Entity, Entity>(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate();
			delete calgo;
		}

		/// Hierarchical Agglomerative Clustering
		if (params->get_algorithm() == 9) {
			class Agglomerative<Entity, Entity> * calgo = new Agglomerative<Entity, Entity>(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate();
			delete calgo;
		}

		/// DBSCAN
		if (params->get_algorithm() == 10) {
			class DBSCAN<Entity, Entity> * calgo = new DBSCAN<Entity, Entity>(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate();
			delete calgo;
		}

		/// kMeans
		if (params->get_algorithm() == 11) {
			class KMeans<Entity, Entity> * calgo = new KMeans<Entity, Entity>(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate();
			delete calgo;
		}
/*
		/// Spectral Clustering
		if (params->get_algorithm() == 12) {
			class SpectralClustering<Entity, Entity> * calgo = new SpectralClustering<Entity, Entity>(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate();
			delete calgo;
		}
*/
		/// Low Dimensional Dominant Vectors (VEPHC)
		if (params->get_algorithm() == 13) {
			class VEPHC * calgo = new VEPHC(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate();
			delete calgo;
		}

		/// Gibbs Sampling on Dirichlet Mixture Model (GSDMM)
		if (params->get_algorithm() == 14) {
			class GSDMM<Entity, Entity> * calgo = new GSDMM<Entity, Entity>(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate();
			delete calgo;
		}

		/// Manual Clustering
		if (params->get_algorithm() == 15) {
			class ManualClustering<Entity, Entity> * calgo = new ManualClustering<Entity, Entity>(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate();
			delete calgo;
		}
	}

	/// ////////////////////////////////////////////////////////////////////////////////////////////
	/// Products Clustering
	if (params->get_entities_type() == 2) {
		/// Token-Based String Similarity Metrics
		if (params->get_algorithm() == 1 || params->get_algorithm() == 2 || params->get_algorithm() == 3 ||
			params->get_algorithm() == 5 || params->get_algorithm() == 6 || params->get_algorithm() == 7) {

			class SimilarityMeasure<Product> * calgo = new SimilarityMeasure<Product>(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate_f1();
			delete calgo;
		}

		/// Edit Distance (Similarity)
		if (params->get_algorithm() == 4) {
			class EditSimilarity<Product> * calgo = new EditSimilarity<Product>(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate_f1();
			delete calgo;
		}

		/// Leader Clustering
		if (params->get_algorithm() == 8) {
			class LeaderClustering< Product, EntitiesGroup<Product> > * calgo = new LeaderClustering< Product, EntitiesGroup<Product> >(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate();
			delete calgo;
		}

		/// Hierarchical Agglomerative Clustering
		if (params->get_algorithm() == 9) {
			class Agglomerative< Product, EntitiesGroup<Product> > * calgo = new Agglomerative< Product, EntitiesGroup<Product> >(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate();
			delete calgo;
		}

		/// DBSCAN
		if (params->get_algorithm() == 10) {
			class DBSCAN< Product, EntitiesGroup<Product> > * calgo = new DBSCAN< Product, EntitiesGroup<Product> >(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate();
			delete calgo;
		}

		/// kMeans
		if (params->get_algorithm() == 11) {
			class KMeans< Product, EntitiesGroup<Product> > * calgo = new KMeans< Product, EntitiesGroup<Product> >(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate();
			delete calgo;
		}
/*
		/// Spectral Clustering
		if (params->get_algorithm() == 12) {
			class SpectralClustering< Product, EntitiesGroup<Product> > * calgo = new SpectralClustering< Product, EntitiesGroup<Product> >(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate();
			delete calgo;
		}
*/
		/// UPM
		if (params->get_algorithm() == 13) {
			class UPM * calgo = new UPM(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate();
			delete calgo;
		}

		/// Gibbs Sampling on Dirichlet Mixture Model (GSDMM)
		if (params->get_algorithm() == 14) {
			class GSDMM< Product, EntitiesGroup<Product> > * calgo = new GSDMM< Product, EntitiesGroup<Product> >(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate();
			delete calgo;
		}

		/// Manual Clustering
		if (params->get_algorithm() == 15) {
			class ManualClustering< Product, EntitiesGroup<Product> > * calgo = new ManualClustering< Product, EntitiesGroup<Product> >(params);
			calgo->read_from_file(filepath);
			calgo->exec();
			calgo->evaluate();
			delete calgo;
		}
	}

	delete params;
	return 0;
}

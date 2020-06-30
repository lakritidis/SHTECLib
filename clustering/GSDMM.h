#ifndef GSDMM_H
#define GSDMM_H

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

template <class T, class U> class GSDMM : public ClusteringAlgorithm<T,U> {
	private:
		uint32_t iterations;
		uint32_t K_clusters;

		_score_t alpha;
		_score_t beta;

	private:
		uint32_t sample(const gsl_rng * r, double *, uint32_t *);
		void score(T *, double *, class Cluster<T> **);
		uint32_t get_num_nonempty_clusters(class Cluster<T> **);

	public:
		GSDMM(class Settings *);
		~GSDMM();

		void exec();
};

#endif // GSDMM_H

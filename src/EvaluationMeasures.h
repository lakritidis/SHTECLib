#ifndef EVALUATIONMEASURES_H
#define EVALUATIONMEASURES_H


class EvaluationMeasures {
	private:
		double duration;
		double precision;
		double recall;
		double f1;
		double nmi;
		double Hy;
		double Hc;
		double Hyc;
		double completeness;
		double homogeinity;
		uint32_t num_algo_matches;
		uint32_t num_algo_clusters;

	public:
		EvaluationMeasures();
		~EvaluationMeasures();

		double get_duration();
		double get_precision();
		double get_recall();
		double get_f1();
		double get_nmi();
		double get_Hy();
		double get_Hc();
		double get_Hyc();
		double get_completeness();
		double get_homogeinity();
		uint32_t get_num_algo_matches();
		uint32_t get_num_algo_clusters();

		void set_duration(double);
		void set_precision(double);
		void set_recall(double);
		void set_f1(double);
		void set_nmi(double);
		void set_Hy(double);
		void set_Hc(double);
		void set_Hyc(double);
		void set_completeness(double);
		void set_homogeinity(double);
		void set_num_algo_matches(uint32_t);
		void set_num_algo_clusters(uint32_t);
		void increase_num_algo_matches();
};

#endif // EVALUATIONMEASURES_H

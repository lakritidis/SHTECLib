#include "EvaluationMeasures.h"

/// Constructor
EvaluationMeasures::EvaluationMeasures() :
	duration(0.0), precision(0.0), recall(0.0), f1(0.0), nmi(0.0), Hy(0.0), Hc(0.0), Hyc(0.0),
	completeness(0.0), homogeinity(0.0), num_algo_matches(0), num_algo_clusters(0) {

}

/// Destructor
EvaluationMeasures::~EvaluationMeasures() {

}

/// Accessors
double EvaluationMeasures::get_duration() { return this->duration; }
double EvaluationMeasures::get_precision() { return this->precision; }
double EvaluationMeasures::get_recall() { return this->recall; }
double EvaluationMeasures::get_f1() { return this->f1; }
double EvaluationMeasures::get_nmi() { return this->nmi; }
double EvaluationMeasures::get_Hy() { return this->Hy; }
double EvaluationMeasures::get_Hc() { return this->Hc; }
double EvaluationMeasures::get_Hyc() { return this->Hyc; }
double EvaluationMeasures::get_completeness() { return this->completeness; }
double EvaluationMeasures::get_homogeinity() { return this->homogeinity; }
uint32_t EvaluationMeasures::get_num_algo_matches() { return this->num_algo_matches; }
uint32_t EvaluationMeasures::get_num_algo_clusters() { return this->num_algo_clusters; }

/// Mutators
void EvaluationMeasures::set_duration(double v) { this->duration = v; }
void EvaluationMeasures::set_precision(double v) { this->precision = v; }
void EvaluationMeasures::set_recall(double v) { this->recall = v; }
void EvaluationMeasures::set_f1(double v) { this->f1 = v; }
void EvaluationMeasures::set_nmi(double v) { this->nmi = v; }
void EvaluationMeasures::set_Hy(double v) { this->Hy = v; }
void EvaluationMeasures::set_Hc(double v) { this->Hc = v; }
void EvaluationMeasures::set_Hyc(double v) { this->Hyc = v; }
void EvaluationMeasures::set_completeness(double v) { this->completeness = v; }
void EvaluationMeasures::set_homogeinity(double v) { this->homogeinity = v; }
void EvaluationMeasures::set_num_algo_clusters(uint32_t v) { this->num_algo_clusters = v; }
void EvaluationMeasures::set_num_algo_matches(uint32_t v) { this->num_algo_matches = v; }
void EvaluationMeasures::increase_num_algo_matches() { this->num_algo_matches++; }

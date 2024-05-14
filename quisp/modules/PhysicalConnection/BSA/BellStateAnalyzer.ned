package modules.PhysicalConnection.BSA;
@namespace(quisp::modules);

simple BellStateAnalyzer
{
    parameters:
        double collection_efficiency = default(1);
        double darkcount_probability = default(0);
        double detection_efficiency = default(1);
        double indistinguishable_time_window @unit(ns) = default(1.5ns); // Precision of photon arrivial time ~1.5ns
        int photon_detection_per_second = default(1000000);
        @signal[clicks_in_batch](type=int);
        @statistic[clicks_in_batch](record=vector);
        @signal[p_arrived](type=int);
        @statistic[p_arrived](record=vector);
        @signal[q_arrived](type=int);
        @statistic[q_arrived](record=vector);
    gates:
        inout quantum_port[];
        output to_bsa_controller;
}

#define NDEBUG
#include <bits/stdc++.h>
#include "cch.h"

using namespace std::chrono;

constexpr double RATIO = 0;
constexpr inline weight_t cost(flow_t flow) {
	return RATIO + (1.0 - RATIO) / (flow + 1);
}

int main(int argc, char** argv) {
	assert(argc == 2);
	std::filesystem::path file(argv[1]);
	std::string name = file.stem();
	std::string basename = name.substr(0, name.find('.'));
	assert(!name.empty());

	std::cout << std::fixed << std::setprecision(6);
	std::cerr << std::fixed << std::setprecision(6);

	std::vector<edge_id> first_out = loadVector<edge_id>("../inputs/first_out");
	std::vector<node_id> head = loadVector<node_id>("../inputs/head");
	std::vector<uint32_t> travel_time = loadVector<uint32_t>("../inputs/travel_time");
	std::vector<weight_t> dist(travel_time.size());
	for (edge_id i = 0; i < travel_time.size(); i++) dist[i] = static_cast<weight_t>(travel_time[i]);
	std::vector<float> lat = loadVector<float>("../inputs/latitude");
	std::vector<float> lng = loadVector<float>("../inputs/longitude");
	std::vector<node_id> node_order = loadVector<node_id>("../inputs/cch_order");
	std::vector<capacity_t> capacity = loadVector<capacity_t>("../inputs/capacity");

	std::vector<node_id> origin = loadVector<node_id>("../inputs/od_pairs/" + basename + ".origin");
	std::vector<node_id> destination = loadVector<node_id>("../inputs/od_pairs/" + basename + ".destination");
	assert(origin.size() == destination.size());

	std::cout << node_order.size() << " " << head.size() << " : " << origin.size() << std::endl;
	auto tail = tail_from_first_out(first_out);

	std::cout << "Building CCH: " << std::flush;
	CCH cch(node_order, tail, head);
	std::cout << cch.cch_arc_count() << std::endl;

	std::cout << "Initial customization" << std::endl;
	CCH_metric metric(cch);
	metric.customize(dist, true);
	CCH_query query(metric);

	std::cout << "start STA" << std::endl;

	std::vector<flow_t> flow;
	std::vector<weight_t> new_dist(dist.size());
	int iteration = 0;
	std::cerr << "iteratiom\ttotal\tpotential" << std::endl;
	double last_potential = -1;
	auto start = high_resolution_clock::now();
	std::vector<double> accumulated_cost(origin.size());
	accumulated_cost[0] = cost(0);
	for (std::size_t i = 1; i < accumulated_cost.size(); i++) {
		accumulated_cost[i] = accumulated_cost[i - 1] + cost(i);
	}
	do {
		iteration++;
		std::cout << "iteration: " << iteration << std::endl;
		std::cout << "  assign od pairs" << std::endl;
		double total = 0;
		for (std::size_t i = 0; i < origin.size(); i++) {
			total += query.query(origin[i], destination[i]);
			query.add_flow(1);
		}
		std::cout << "  total: " << total << std::endl;
		
		std::cout << "  calculate flow/new travel time" << std::endl;
		flow = metric.get_flow();
		double potential = 0;
		for (edge_id i = 0; i < cch.input_arc_count(); i++) {
			new_dist[i] = dist[i] * cost(flow[i]);
			assert(new_dist[i] <= dist[i]);
			potential += accumulated_cost[flow[i]];
		}
		std::cout << "  potential: " << potential << std::endl;
		std::cerr << iteration << "\t" << total << "\t" << potential << std::endl;

		//dumpVector<flow_t>("../outputs/" + basename + ".cap." + std::to_string(iteration) + ".flow", flow);

		if (abs(last_potential - potential) < 0.000001) break;
		last_potential = potential;
		std::cout << "  preparing next iteration" << std::endl;
		metric.customize(new_dist, true);
	} while (true);
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);

	dumpVector<flow_t>("../outputs/" + basename + ".cap.flow", flow);

	std::cout << "iterations: " << iteration << std::endl;
	std::cout << "total: " << duration.count() << " microseconds" << std::endl;

}

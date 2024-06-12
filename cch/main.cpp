//#define NDEBUG
#include <bits/stdc++.h>
#include "cch.h"

using namespace std::chrono;

int main(int /*argc*/, char** /*argv*/) {
	std::cout << std::fixed << std::setprecision(6);

	std::vector<edge_id> first_out = loadVector<edge_id>("../inputs/first_out");
	std::vector<node_id> head = loadVector<node_id>("../inputs/head");
	std::vector<uint32_t> travel_time = loadVector<uint32_t>("../inputs/travel_time");
	std::vector<weight_t> dist(travel_time.size());
	for (edge_id i = 0; i < travel_time.size(); i++) dist[i] = static_cast<weight_t>(travel_time[i]);
	std::vector<float> lat = loadVector<float>("../inputs/latitude");
	std::vector<float> lng = loadVector<float>("../inputs/longitude");
	std::vector<node_id> node_order = loadVector<node_id>("../inputs/cch_order");
	// capacity?
	std::vector<node_id> origin = loadVector<node_id>("../inputs/od_pairs/Stuttgart_morn.origin");
	std::vector<node_id> destination = loadVector<node_id>("../inputs/od_pairs/Stuttgart_morn.destination");
	assert(origin.size() == destination.size());

	std::cout << node_order.size() << " " << head.size() << std::endl;
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
			new_dist[i] = dist[i] / (flow[i] + 1);
			assert(new_dist[i] <= dist[i]);
			for (flow_t commodity = 0; commodity <= flow[i]; commodity++) {
				potential += static_cast<double>(dist[i]) / (commodity + 1);
			}
		}
		std::cout << "  potential: " << potential << std::endl;
		std::cerr << iteration << "\t" << total << "\t" << potential << std::endl;
		//todo check convergence
		if (false) break;
		std::cout << "  preparing next iteration" << std::endl;
		metric.customize(new_dist, true);
	} while (true);

}

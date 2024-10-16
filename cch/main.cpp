#define NDEBUG
#include <bits/stdc++.h>
#include "cch.h"

using namespace std::chrono;

constexpr double RATIO = 0.0;
constexpr double DIV = 1.0;//1.0;
constexpr inline weight_t cost(flow_t flow) {
	return RATIO + (1.0 - RATIO) / ((flow / DIV) + 1);
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

	std::cout << "Graph size: " << node_order.size() << " " << head.size() << ", O-D-Pairs: " << origin.size() << std::endl;
	auto tail = tail_from_first_out(first_out);

	std::cout << "Building CCH: " << std::flush;
	CCH cch(node_order, tail, head);
	std::cout << cch.cch_arc_count() << std::endl;

	std::cout << "Initial customization" << std::endl;
	CCH_metric metric(cch);
	metric.customize(dist, true);
	CCH_query query(metric);


	std::vector<flow_t> flow;
	std::vector<weight_t> new_dist(dist.size());
	int iteration = 0;

	/* only used to calculate potential
	double last_potential = -1;
	std::vector<double> accumulated_cost(origin.size() + 1);
	accumulated_cost[0] = cost(0);
	for (std::size_t i = 1; i < accumulated_cost.size(); i++) {
		accumulated_cost[i] = accumulated_cost[i - 1] + cost(i);
	}*/


	std::cout << "start STA" << std::endl;
	auto start = high_resolution_clock::now();
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
		
		std::vector<flow_t> new_flow = metric.get_flow();
		if (new_flow == flow) break;
		flow = std::move(new_flow);

		std::cout << "  calculate flow/new travel time" << std::endl;	
		//double potential = 0;
		for (edge_id i = 0; i < cch.input_arc_count(); i++) {
			new_dist[i] = dist[i] * cost(flow[i]);
			assert(new_dist[i] <= dist[i]);
			//potential += accumulated_cost[flow[i]];
		}
		/*std::cout << "  potential: " << potential << std::endl;
		std::cerr << iteration << "\t" << total << "\t" << potential << std::endl;
		last_potential = potential;*/

		//dumpVector<flow_t>("../outputs/" + basename + ".ratio." + std::to_string(RATIO) + ".div." + std::to_string(DIV) + "." + std::to_string(iteration) + ".flow", flow);

		std::cout << "  preparing next iteration" << std::endl;
		metric.customize(new_dist, true);
	} while (true);
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);

	//dumpVector<flow_t>("../outputs/" + basename + ".cap.flow", flow);

	std::cout << "iterations: " << iteration << std::endl;
	std::cout << "total: " << duration.count() << " microseconds" << std::endl;

	//eval
	CCH_metric base_metric(cch);
	base_metric.customize(dist, true);
	CCH_query base_query(base_metric);

	double stretch = 0;
	double sharing = 0;
	int nonZero = 0;
	for (std::size_t i = 0; i < origin.size(); i++) {
		if (origin[i] == destination[i]) continue;
		nonZero++;
		weight_t base_dist = base_query.query(origin[i], destination[i]);
		query.query(origin[i], destination[i]);
		auto path = query.path();

		double cur_sharing = 0;
		weight_t cur_dist = 0;
		for (std::size_t j = 0; j + 1 < path.size(); j++) {
			node_id f = path[j];
			node_id t = path[j + 1];
			
			edge_id e = invalid_id;
			for (edge_id k = first_out[f]; k < first_out[f + 1]; k++) {
				if (head[k] == t) {
					if (e == invalid_id || dist[k] < dist[e]) {
						e = k;
					}
				}
			}

			assert(e != invalid_id);
			cur_dist += dist[e];
			cur_sharing += (double)dist[e] * (flow[e] - 1);
		}
		stretch += (double)cur_dist / base_dist;
		sharing += cur_sharing / cur_dist;
	}

	//given x in [0, 1] and fixed k (1) -> y = %of od-pairs that share at least x of their path with (at least) k other people
	int k = 200;
	std::map<double, int> tmp;
	for (std::size_t i = 0; i < origin.size(); i++) {
		if (origin[i] == destination[i]) {
			tmp[1.0]++;
		} else {
			query.query(origin[i], destination[i]);
			auto path = query.path();
			weight_t cur_dist = 0;
			weight_t shared = 0;
			for (std::size_t j = 0; j + 1 < path.size(); j++) {
				node_id f = path[j];
				node_id t = path[j + 1];
				
				edge_id e = invalid_id;
				for (edge_id k = first_out[f]; k < first_out[f + 1]; k++) {
					if (head[k] == t) {
						if (e == invalid_id || dist[k] < dist[e]) {
							e = k;
						}
					}
				}

				assert(e != invalid_id);
				if (flow[e] > k) shared += dist[e];
				cur_dist += dist[e];
			}
			tmp[1.0 * shared / cur_dist]++;
		}
	}
	std::map<double, int> res;
	for (auto [x, y] : tmp) {
		//group x positions
		res[round(x * 1000) / 1000] += y;
	}
	std::cerr << "x,y" << std::endl;
	int last = 0;
	for (auto it = res.rbegin(); it != res.rend(); it++) {
		it->second += last;//suffix sum
		last = it->second;
		std::cerr << it->first << "," << 1.0*last / origin.size() << std::endl;
	}
}

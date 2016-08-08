#include <clingo.hh>

#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

constexpr auto maxStep = 100;

Clingo::SymbolVector symbols;

void groundHorizon(Clingo::Control& control, const Clingo::PartSpan& horizonParts, const Clingo::Symbol& query) noexcept {
	control.ground(horizonParts);
	control.assign_external(query, Clingo::TruthValue::True);
	return;
}

bool onModel(const Clingo::Model& newModel) noexcept {
	symbols = newModel.symbols(Clingo::ShowType::All);
	return false;
}

void blank(void) noexcept {
	for ( int step = 1; step <= maxStep; ++step ) {
		Clingo::Control control;
		
		control.load("../program/graph_wg.lp");
		control.load("../program/mailbot.lp");
		
		control.ground({{"base", Clingo::SymbolSpan()}});
		
		Clingo::Symbol horizon(Clingo::Number(0)), stepSymbol(Clingo::Number(1));
		Clingo::SymbolSpan horizonSpan(&horizon, 1), stepSpan(&stepSymbol, 1);
		Clingo::PartSpan horizonParts{{"state", horizonSpan}, {"transition", horizonSpan}, {"query", horizonSpan}};
		Clingo::PartSpan finalizeParts{{"finalize", stepSpan}};
		
		Clingo::Symbol query(Clingo::Function("query", horizonSpan, true));
		
		groundHorizon(control, horizonParts, query);
		for ( int tempStep = 1; tempStep <= step; ++ tempStep ) {
			stepSymbol = Clingo::Number(tempStep);
			control.ground(finalizeParts);
		} //for ( int tempStep = 1; tempStep <= step; ++ tempStep )
		
		control.solve(onModel);
	} //for ( int step = 1; step <= maxStep; ++step )
	return;
}

void blankReactive(void) noexcept {
	Clingo::Control control;
	
	control.load("../program/graph_wg.lp");
	control.load("../program/mailbot.lp");
	
	control.ground({{"base", Clingo::SymbolSpan()}});
	
	Clingo::Symbol horizon(Clingo::Number(0)), stepSymbol(Clingo::Number(1));
	Clingo::SymbolSpan horizonSpan(&horizon, 1), stepSpan(&stepSymbol, 1);
	Clingo::PartSpan horizonParts{{"state", horizonSpan}, {"transition", horizonSpan}, {"query", horizonSpan}};
	Clingo::PartSpan finalizeParts{{"finalize", stepSpan}};
	
	Clingo::Symbol query(Clingo::Function("query", horizonSpan, true));
	
	groundHorizon(control, horizonParts, query);
	
	for ( int step = 1; step <= maxStep; ++step ) {
		stepSymbol = Clingo::Number(step);
		control.ground(finalizeParts);
		control.solve(onModel);
	} //for ( int step = 1; step <= maxStep; ++step )
	return;
}

void events(void) noexcept {
	
	return;
}

void eventsReactive(void) noexcept {
	
	return;
}

int main(const int argc, const char *argv[]) {
	using clock = std::chrono::high_resolution_clock;
	using duration = std::chrono::nanoseconds;
	std::vector<duration> durations[4];
	
	const int max = argc >= 2 ? std::max(1, std::atoi(argv[1])) : 5;
	
	auto run = [](std::vector<duration>& vec, const std::function<void(void)>& fun) noexcept {
			const auto begin = clock::now();
			fun();
			vec.push_back(clock::now() - begin);
			return;
		};
	
	for ( auto i = 1; i <= max; ++i ) {
		std::cout<<"Starting iteration #"<<i<<" of "<<max<<std::endl;
		run(durations[0], blank);
		std::cout<<"\tBlank done."<<std::endl;
		run(durations[1], blankReactive);
		std::cout<<"\tBlank-R done."<<std::endl;
		run(durations[2], events);
		std::cout<<"\tEvents done."<<std::endl;
		run(durations[3], eventsReactive);
		std::cout<<"\tEvents-R done."<<std::endl;
	} //for ( auto i = 1; i <= max; ++i )
	
	std::cout<<std::endl;
	
	auto out = [max](const std::vector<duration>& vec, const char *name) noexcept {
			const auto sum = std::accumulate(vec.begin(), vec.end(), duration());
			const auto avg = sum / max;
			std::cout<<std::setw(15)<<name<<": "<<(avg.count() / 1000000.)<<" s"<<std::endl;
			return;
		};
	
	out(durations[0], "Blank");
	out(durations[1], "Blank-Reactive");
	out(durations[2], "Events");
	out(durations[3], "Events-Reactive");
	return 0;
}

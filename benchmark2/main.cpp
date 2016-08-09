#include <clingo.hh>

#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

constexpr auto maxStep = 100;

Clingo::SymbolVector symbols;

void groundHorizon(Clingo::Control& control, Clingo::Control& reactiveControl, const Clingo::PartSpan& horizonParts, const Clingo::Symbol& query) noexcept {
	control.ground(horizonParts);
	control.assign_external(query, Clingo::TruthValue::True);
	reactiveControl.ground(horizonParts);
	reactiveControl.assign_external(query, Clingo::TruthValue::True);
	return;
}

void groundHorizon(Clingo::Control& control, const Clingo::PartSpan& horizonParts, const Clingo::Symbol& query) noexcept {
	control.ground(horizonParts);
	control.assign_external(query, Clingo::TruthValue::True);
	return;
}

bool onModel(const Clingo::Model& newModel) noexcept {
	symbols = newModel.symbols(Clingo::ShowType::All);
	return false;
}

bool onModelNoOp(const Clingo::Model&) noexcept {
	return false;
}

void setHorizon(Clingo::Control& control, Clingo::Control& reactiveControl, Clingo::Symbol& query, const Clingo::PartSpan& horizonParts, Clingo::Symbol& horizon, const Clingo::SymbolSpan& horizonSpan, const int newHorizon) noexcept {
	control.assign_external(query, Clingo::TruthValue::False);
	reactiveControl.assign_external(query, Clingo::TruthValue::False);
	horizon = Clingo::Number(newHorizon);
	query = Clingo::Function("query", horizonSpan, true);
	groundHorizon(control, reactiveControl, horizonParts, query);
	return;
}

void setHorizon(Clingo::Control& control, Clingo::Symbol& query, const Clingo::PartSpan& horizonParts, Clingo::Symbol& horizon, const Clingo::SymbolSpan& horizonSpan, const int newHorizon) noexcept {
	control.assign_external(query, Clingo::TruthValue::False);
	horizon = Clingo::Number(newHorizon);
	query = Clingo::Function("query", horizonSpan, true);
	groundHorizon(control, horizonParts, query);
	return;
}

void incrHorizon(Clingo::Control& control, Clingo::Control& reactiveControl, Clingo::Symbol& query, const Clingo::PartSpan& horizonParts, Clingo::Symbol& horizon, const Clingo::SymbolSpan& horizonSpan) noexcept {
	setHorizon(control, reactiveControl, query, horizonParts, horizon, horizonSpan, horizon.number() + 1);
	return;
}

void incrHorizon(Clingo::Control& control, Clingo::Symbol& query, const Clingo::PartSpan& horizonParts, Clingo::Symbol& horizon, const Clingo::SymbolSpan& horizonSpan) noexcept {
	setHorizon(control, query, horizonParts, horizon, horizonSpan, horizon.number() + 1);
	return;
}

bool parse(Clingo::Control& control, Clingo::Control& reactiveControl, const int step) noexcept {
	bool ret = false;
	for ( const Clingo::Symbol& s : symbols ) {
		const auto args(s.arguments());
		if ( args.size() == 3 && std::strcmp(s.name(), "do") == 0 && args[2].number() == step ) {
			ret = true;
			const Clingo::PartSpan commitSpan{{"commit", args}};
			control.ground(commitSpan);
			reactiveControl.ground(commitSpan);
			
			const Clingo::SymbolSpan setEventArgs{args[0], Clingo::Id("success"), args[2]};
			const Clingo::PartSpan setEventSpan{{"set_event", setEventArgs}};
			control.ground(setEventSpan);
			reactiveControl.ground(setEventSpan);
		} //if ( args.size() == 3 && std::strcmp(s.name(), "do") == 0 && args[2].number() == step )
	} //for ( const Clingo::Symbol& s : symbols )
	return ret;
}

bool addEvent(Clingo::Control& control, Clingo::Control& reactiveControl, const int step) noexcept {
	int jobID;
	Clingo::Symbol job;
	
	auto bring = [&job](const char *o1, const char *o2) noexcept {
			job = Clingo::Function("bring", {Clingo::Id(o1), Clingo::Id(o2)});
			return;
		};
	
	auto cancel = [&job](void) noexcept {
			job = Clingo::Id("cancel");
			return;
		};
	
	switch ( step ) {
		case  1 : jobID = 1; bring("o9", "o12"); break;
		case  4 : jobID = 2; bring("o7", "o5");  break;
		case  8 : jobID = 3; bring("o1", "o2");  break;
		case  9 : jobID = 3; cancel();           break;
		case 22 : jobID = 1; bring("o12", "o9"); break;
		case 24 : jobID = 2; bring("o5", "o11"); break;
		case 36 : jobID = 2; cancel();           break;
		case 37 : jobID = 3; bring("o2", "o8");  break;
		case 40 : jobID = 1; bring("o3", "o8");  break;
		case 42 : jobID = 2; bring("o11", "o4"); break;
		case 64 : jobID = 3; bring("o6", "o10"); break;
		case maxStep : jobID = 1; bring("o3", "o4"); break;
		default : return false;
	} //switch ( step )
	
	const Clingo::SymbolSpan parameters{Clingo::Number(jobID), job, Clingo::Number(step)};
	const Clingo::Part part("set_event", parameters);
	const Clingo::PartSpan parts(&part, 1);
	control.ground(parts);
	reactiveControl.ground(parts);
	return true;
}

int main(const int argc, const char *argv[]) {
	using clock = std::chrono::high_resolution_clock;
	using duration = std::chrono::nanoseconds;
	std::vector<duration> durations[2];
	
	const int max = argc >= 2 ? std::max(1, std::atoi(argv[1])) : 5;
	
	for ( auto i = 1; i <= max; ++i ) {
		std::cout<<"Starting iteration #"<<i<<" of "<<max<<std::endl;
		
		Clingo::Control control, reactiveControl;
		symbols.clear();
		
		control.load("../program/graph_wg.lp");
		control.load("../program/mailbot.lp");
		reactiveControl.load("../program/graph_wg.lp");
		reactiveControl.load("../program/mailbot.lp");
		
		control.ground({{"base", Clingo::SymbolSpan()}});
		reactiveControl.ground({{"base", Clingo::SymbolSpan()}});
		
		Clingo::Symbol horizon(Clingo::Number(0));
		Clingo::SymbolSpan horizonSpan(&horizon, 1);
		Clingo::Part horizonPartsArray[3] = {{"state", horizonSpan}, {"transition", horizonSpan}, {"query", horizonSpan}};
		Clingo::PartSpan horizonParts(horizonPartsArray, 3);
		
		Clingo::Symbol query(Clingo::Function("query", horizonSpan, true));
		
		groundHorizon(control, reactiveControl, horizonParts, query);
		
		for ( int step = 1; step < maxStep; ++step ) {
			if ( addEvent(control, reactiveControl, step) ) {
				while ( horizon.number() < step ) {
					incrHorizon(control, reactiveControl, query, horizonParts, horizon, horizonSpan);
				} //while ( horizon.number() < step )
				while ( reactiveControl.solve(onModel).is_unsatisfiable() ) {
					incrHorizon(control, reactiveControl, query, horizonParts, horizon, horizonSpan);
				} //while ( reactiveControl.solve(onModel).is_unsatisfiable() )
			} //if ( addEvent(control, reactiveControl, step) )
			if ( parse(control, reactiveControl, step) ) {
				reactiveControl.solve(onModel);
			} //if ( parse(control, reactiveControl, step) )
		} //for ( int step = 1; step < maxStep; ++step )
		
		while ( horizon.number() < maxStep ) {
			incrHorizon(control, reactiveControl, query, horizonParts, horizon, horizonSpan);
		} //while ( horizon.number() < maxStep )
		addEvent(control, reactiveControl, maxStep);
		
		Clingo::Symbol reactiveHorizon(horizon);
		Clingo::SymbolSpan reactiveHorizonSpan(&reactiveHorizon, 1);
		Clingo::Part reactiveHorizonPartsArray[3] = {{"state", reactiveHorizonSpan}, {"transition", reactiveHorizonSpan}, {"query", reactiveHorizonSpan}};
		Clingo::PartSpan reactiveHorizonParts(reactiveHorizonPartsArray, 3);
		
		Clingo::Symbol reactiveQuery(Clingo::Function("query", reactiveHorizonSpan, true));
		
		auto run = [](std::vector<duration>& vec, Clingo::Control& control, Clingo::Symbol& horizon, Clingo::PartSpan& horizonParts, Clingo::SymbolSpan& horizonSpan, Clingo::Symbol& query) noexcept {
				const auto begin = clock::now();
				while ( control.solve(onModel).is_unsatisfiable() ) {
					incrHorizon(control, query, horizonParts, horizon, horizonSpan);
				} //while ( control.solve(onModel).is_unsatisfiable() )
				control.solve(onModelNoOp);
				vec.push_back(clock::now() - begin);
				return;
			};
		
		run(durations[0], control, horizon, horizonParts, horizonSpan, query);
		run(durations[1], reactiveControl, reactiveHorizon, reactiveHorizonParts, reactiveHorizonSpan, reactiveQuery);
	} //for ( auto i = 1; i <= max; ++i )
	
	std::cout<<std::endl;
	
	auto out = [max](const std::vector<duration>& vec, const char *name) noexcept {
			const auto sum = std::accumulate(vec.begin(), vec.end(), duration());
			const auto avg = sum / max;
			std::cout<<std::setw(9)<<name<<": "<<(avg.count() / 1000000.)<<" s"<<std::endl;
			return;
		};
	
	out(durations[0], "Normal");
	out(durations[1], "Reactive");
	return 0;
}

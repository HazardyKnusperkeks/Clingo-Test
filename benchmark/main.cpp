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

bool onModelNoOp(const Clingo::Model&) noexcept {
	return false;
}

void blank(void) noexcept {
	symbols.clear();
	
	for ( int step = 1; step <= maxStep; ++step ) {
		Clingo::Control control;
		
		control.load("../program/graph_wg.lp");
		control.load("../program/mailbot.lp");
		
		control.ground({{"base", Clingo::SymbolSpan()}});
		
		Clingo::Symbol horizon(Clingo::Number(0)), stepSymbol(Clingo::Number(1));
		Clingo::SymbolSpan horizonSpan(&horizon, 1), stepSpan(&stepSymbol, 1);
		Clingo::Part horizonPartsArray[3] = {{"state", horizonSpan}, {"transition", horizonSpan}, {"query", horizonSpan}};
		Clingo::PartSpan horizonParts(horizonPartsArray, 3);
		Clingo::PartSpan finalizeParts{{"finalize", stepSpan}};
		
		Clingo::Symbol query(Clingo::Function("query", horizonSpan, true));
		
		groundHorizon(control, horizonParts, query);
		for ( int tempStep = 1; tempStep <= step; ++tempStep ) {
			stepSymbol = Clingo::Number(tempStep);
			control.ground(finalizeParts);
		} //for ( int tempStep = 1; tempStep <= step; ++tempStep )
		
		control.solve(onModel);
	} //for ( int step = 1; step <= maxStep; ++step )
	return;
}

void blankReactive(void) noexcept {
	Clingo::Control control;
	symbols.clear();
	
	control.load("../program/graph_wg.lp");
	control.load("../program/mailbot.lp");
	
	control.ground({{"base", Clingo::SymbolSpan()}});
	
	Clingo::Symbol horizon(Clingo::Number(0)), stepSymbol(Clingo::Number(1));
	Clingo::SymbolSpan horizonSpan(&horizon, 1), stepSpan(&stepSymbol, 1);
	Clingo::Part horizonPartsArray[3] = {{"state", horizonSpan}, {"transition", horizonSpan}, {"query", horizonSpan}};
	Clingo::PartSpan horizonParts(horizonPartsArray, 3);
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

void setHorizon(Clingo::Control& control, Clingo::Symbol& query, const Clingo::PartSpan& horizonParts, Clingo::Symbol& horizon, const Clingo::SymbolSpan& horizonSpan, const int newHorizon) noexcept {
	control.assign_external(query, Clingo::TruthValue::False);
	horizon = Clingo::Number(newHorizon);
	query = Clingo::Function("query", horizonSpan, true);
	groundHorizon(control, horizonParts, query);
	return;
}

void incrHorizon(Clingo::Control& control, Clingo::Symbol& query, const Clingo::PartSpan& horizonParts, Clingo::Symbol& horizon, const Clingo::SymbolSpan& horizonSpan) noexcept {
	setHorizon(control, query, horizonParts, horizon, horizonSpan, horizon.number() + 1);
	return;
}

bool parse(Clingo::Control& control, const int step) noexcept {
	bool ret = false;
	for ( const Clingo::Symbol& s : symbols ) {
		const auto args(s.arguments());
		if ( args.size() == 3 && std::strcmp(s.name(), "do") == 0 && args[2].number() == step ) {
			ret = true;
			control.ground({{"commit", args}});
			
			const Clingo::SymbolSpan setEventArgs{args[0], Clingo::Id("success"), args[2]};
			control.ground({{"set_event", setEventArgs}});
		} //if ( args.size() == 3 && std::strcmp(s.name(), "do") == 0 && args[2].number() == step )
	} //for ( const Clingo::Symbol& s : symbols )
	return ret;
}

bool addEvent(Clingo::Control& control, const int step) noexcept {
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
		default : return false;
	} //switch ( step )
	
	Clingo::SymbolSpan parameters{Clingo::Number(jobID), job, Clingo::Number(step)};
	control.ground({{"set_event", parameters}});
	return true;
}

void events(void) noexcept {
	symbols.clear();
	int oldHorizon = 0;
	
	for ( int step = 1; step <= maxStep; ++step ) {
		Clingo::Control control;
		
		control.load("../program/graph_wg.lp");
		control.load("../program/mailbot.lp");
		
		control.ground({{"base", Clingo::SymbolSpan()}});
		
		Clingo::Symbol horizon(Clingo::Number(0));
		Clingo::SymbolSpan horizonSpan(&horizon, 1);
		Clingo::Part horizonPartsArray[3] = {{"state", horizonSpan}, {"transition", horizonSpan}, {"query", horizonSpan}};
		Clingo::PartSpan horizonParts(horizonPartsArray, 3);
		
		Clingo::Symbol query(Clingo::Function("query", horizonSpan, true));
		
		groundHorizon(control, horizonParts, query);
		
		for ( int tempStep = 1; tempStep <= step; ++tempStep ) {
			if ( tempStep < oldHorizon && horizon.number() < tempStep ) {
				setHorizon(control, query, horizonParts, horizon, horizonSpan, tempStep);
			} //if ( tempStep < oldHorizon && horizon.number() < tempStep )
			if ( addEvent(control, tempStep) ) {
				while ( horizon.number() < tempStep ) {
					incrHorizon(control, query, horizonParts, horizon, horizonSpan);
				} //while ( horizon.number() < tempStep )
				while ( control.solve(onModelNoOp).is_unsatisfiable() ) {
					incrHorizon(control, query, horizonParts, horizon, horizonSpan);
				} //while ( control.solve(onModelNoOp).is_unsatisfiable() )
			} //if ( addEvent(control, tempStep) )
			if ( parse(control, tempStep) ) {
				control.solve(onModelNoOp);
			} //if ( parse(control, tempStep) )
		} //for ( int tempStep = 1; tempStep <= step; ++tempStep )
		
		control.solve(onModel);
		oldHorizon = horizon.number();
	} //for ( int step = 1; step <= maxStep; ++step )
	return;
}

void eventsReactive(void) noexcept {
	Clingo::Control control;
	symbols.clear();
	
	control.load("../program/graph_wg.lp");
	control.load("../program/mailbot.lp");
	
	control.ground({{"base", Clingo::SymbolSpan()}});
	
	Clingo::Symbol horizon(Clingo::Number(0));
	Clingo::SymbolSpan horizonSpan(&horizon, 1);
	Clingo::Part horizonPartsArray[3] = {{"state", horizonSpan}, {"transition", horizonSpan}, {"query", horizonSpan}};
	Clingo::PartSpan horizonParts(horizonPartsArray, 3);
	
	Clingo::Symbol query(Clingo::Function("query", horizonSpan, true));
	
	groundHorizon(control, horizonParts, query);
	
	for ( int step = 1; step <= maxStep; ++step ) {
		
		if ( addEvent(control, step) ) {
			while ( horizon.number() < step ) {
				incrHorizon(control, query, horizonParts, horizon, horizonSpan);
			} //while ( horizon.number() < step )
			while ( control.solve(onModel).is_unsatisfiable() ) {
				incrHorizon(control, query, horizonParts, horizon, horizonSpan);
			} //while ( control.solve(onModel).is_unsatisfiable() )
		} //if ( addEvent(control, step) )
		if ( parse(control, step) ) {
			control.solve(onModel);
		} //if ( parse(control, step) )
		
	} //for ( int step = 1; step <= maxStep; ++step )
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

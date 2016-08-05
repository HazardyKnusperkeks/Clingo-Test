#include <clingo.hh>

#define QT_NO_DEBUG_OUTPUT

#include <QApplication>
#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QPushButton>
#include <QStringListModel>

QDebug operator<<(QDebug d, const Clingo::Symbol& s) noexcept {
	d.nospace()<<s.to_string().c_str();
	return d;
}

QDebug operator<<(QDebug d, const Clingo::SymbolSpan& s) noexcept {
	d.nospace()<<'[';
	
	const int size = s.size();
	if ( size > 0 ) {
		for ( int i = 0; i < size - 1; ++i ) {
			d.nospace()<<s[i]<<", ";
		} //for ( int i = 0; i < size - 1; ++i )
		d.nospace()<<s.back();
	} //if ( size > 0 )
	
	d.nospace()<<']';
	return d;
}

QDebug operator<<(QDebug d, const Clingo::Part& p) noexcept {
	d.nospace()<<p.name()<<": "<<p.params();
	return d;
}

QDebug operator<<(QDebug d, const Clingo::PartSpan& s) noexcept {
	d.nospace()<<'['<<endl;
	
	const int size = s.size();
	if ( size > 0 ) {
		for ( int i = 0; i < size; ++i ) {
			d.nospace()<<'\t'<<s[i]<<endl;
		} //for ( int i = 0; i < size; ++i )
	} //if ( size > 0 )
	
	d.nospace()<<']';
	return d;
}

QDebug operator<<(QDebug d, const Clingo::SolveResult& res) noexcept {
	QStringList list;
	if ( res.is_satisfiable() ) {
		list.append("SAT");
	} //if ( res.is_satisfiable() )
	if ( res.is_unsatisfiable() ) {
		list.append("UNSAT");
	} //if ( res.is_unsatisfiable() )operator
	if ( res.is_interrupted() ) {
		list.append("INT");
	} //if ( res.is_interrupted() )
	if ( res.is_exhausted() ) {
		list.append("EX");
	} //if ( res.is_exhausted() )
	d<<list.join(" | ");
	return d;
}

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	
	Clingo::Control control;
	Clingo::SymbolVector symbols;
	
	control.load("../program/graph_wg.lp");
	control.load("../program/mailbot.lp");
	
	qDebug()<<"Grounding:"<<"base"<<Clingo::SymbolSpan();
	control.ground({{"base", Clingo::SymbolSpan()}});
	
	Clingo::Symbol horizon(Clingo::Number(0)), step(Clingo::Number(1));
	Clingo::SymbolSpan horizonSpan(&horizon, 1), stepSpan(&step, 1);
	Clingo::PartSpan horizonParts{{"state", horizonSpan}, {"transition", horizonSpan}, {"query", horizonSpan}};
	Clingo::PartSpan stepParts{{"finalize", stepSpan}};
	
	Clingo::Symbol query(Clingo::Function("query", horizonSpan, true));
	
	auto groundHorizon = [&](void) noexcept {
			qDebug()<<"Grounding:"<<horizonParts;
			control.ground(horizonParts);
			control.assign_external(query, Clingo::TruthValue::True);
			return;
		};
	
	groundHorizon();
	
	QWidget widget;
	QGridLayout layout(&widget);
	QLabel status(&widget);
	QLabel result(&widget);
	QLabel horizonLabel("Horizon: 0", &widget);
	QLabel stepLabel("Step: 1", &widget);
	QPushButton nextStepButton("Next Step", &widget);
	QLineEdit edit(&widget);
	QPushButton add("Add", &widget);
	QListView view(&widget);
	QStringListModel model;
	
	view.setModel(&model);
	
	layout.addWidget(&status,         0, 0);
	layout.addWidget(&result,         0, 1);
	layout.addWidget(&horizonLabel,   1, 0);
	layout.addWidget(&stepLabel,      1, 1);
	layout.addWidget(&nextStepButton, 1, 2);
	layout.addWidget(&edit,           2, 0, 1,  2);
	layout.addWidget(&add,            2, 2);
	layout.addWidget(&view,           3, 0, 1, -1);
	
	auto incrHorizon = [&](void) noexcept {
			control.assign_external(query, Clingo::TruthValue::False);
			const int newHorizon = horizon.number() + 1;
			horizonLabel.setText("Horizon: " + QString::number(newHorizon));
			horizon = Clingo::Number(newHorizon);
			query = Clingo::Function("query", horizonSpan, true);
			groundHorizon();
			return;
		};
	
	auto parse = [&](const int step) noexcept {
			
			return;
		};
	
	auto onModel = [&](const Clingo::Model& newModel) noexcept {
			symbols = newModel.symbols(Clingo::ShowType::All);
			QStringList list;
			list.reserve(symbols.size());
			for ( const Clingo::Symbol& s : symbols ) {
				list.append(s.to_string().c_str());
			} //for ( const Clingo::Symbol& s : symbols )
			model.setStringList(list);
			return false;
		};
	
	auto solve = [&](void) noexcept {
			status.setText("Solving");
			result.setText(QString());
			Clingo::SolveResult res(control.solve(onModel));
			qDebug()<<res;
			while ( res.is_unsatisfiable() ) {
				result.setText("Unsatisfied");
				incrHorizon();
				res = control.solve(onModel);
				qDebug()<<res;
			} //while ( res.is_unsatisfiable() )
			result.setText("Satisfied");
			status.setText("Finished");
			return;
		};
	
	auto nextStep = [&](void) noexcept {
			bool haveToSolve = false;
//			qDebug()<<"Grounding:"<<stepParts;
//			control.ground(stepParts);
			const int currentStep = step.number(), newStep = currentStep + 1;
			parse(currentStep);
			stepLabel.setText("Step: " + QString::number(newStep));
			step = Clingo::Number(newStep);
			if ( haveToSolve ) {
				solve();
			} //if ( haveToSolve )
			return;
		};
	
	solve();
	widget.show();
	
	QObject::connect(&nextStepButton, &QPushButton::clicked, nextStep);
	QObject::connect(&add,            &QPushButton::clicked, [&](void) noexcept {
			const QStringList text(edit.text().split(' '));
			edit.clear();
			Clingo::SymbolSpan parameters;
			
			switch ( text.size() ) {
				case 1 : {
					
					break;
				} //case 1
				case 3 : {
					Clingo::Symbol bringArgs[2];
					for ( int i = 0; i < 2; ++i ) {
						bringArgs[i] = Clingo::Id(text.at(i).toStdString().c_str());
					} //for ( int i = 0; i < 2; ++i )
					
					Clingo::Symbol bring = Clingo::Function("bring", {bringArgs, 2});
					parameters = {Clingo::Number(1), bring, step};
					break;
				} //case 3
				default : break; //Print error
			} //switch ( text.size() )
			if ( parameters.size() == 3 ) {
				const int targetHorizon = parameters[2].number();
				while ( horizon.number() < targetHorizon ) {
					incrHorizon();
				} //while ( horizon.number() < targetHorizon )
				qDebug()<<"Grounding:"<<"set_event"<<parameters;
				control.ground({{"set_event", parameters}});
				solve();
			} //if ( parameters.size() == 3 )
			return;
		});
	
	return app.exec();
}

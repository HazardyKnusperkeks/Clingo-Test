#include <clingo.hh>

#include <QApplication>
#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QPushButton>
#include <QStringListModel>

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	
	Clingo::Control control;
	Clingo::SymbolVector symbols;
	
	control.load("../program/graph_wg.lp");
	control.load("../program/mailbot.lp");
	
	control.ground({{"base", Clingo::SymbolSpan()}});
	
	Clingo::Symbol horizon(Clingo::Number(0)), step(horizon);
	Clingo::SymbolSpan horizonSpan(&horizon, 1), stepSpan(&step, 1);
	Clingo::PartSpan horizonParts{{"state", horizonSpan}, {"transition", horizonSpan}, {"query", horizonSpan}};
	Clingo::PartSpan stepParts{{"finalize", stepSpan}};
	
	Clingo::Symbol query(Clingo::Function("query", horizonSpan, true));
	
	auto groundHorizon = [&](void) noexcept {
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
	QLabel stepLabel("Step: 0", &widget);
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
			while ( control.solve(onModel).is_unsatisfiable() ) {
				result.setText("Unsatisfied");
				incrHorizon();
			} //while ( control.solve(onModel).is_unsatisfiable() )
			result.setText("Satisfied");
			status.setText("Finished");
			return;
		};
	
	auto nextStep = [&](void) noexcept {
			bool haveToSolve = false;
			control.ground(stepParts);
			if ( step >= horizon ) {
				haveToSolve = true;
				incrHorizon();
			} //if ( step >= horizon )
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
			
			return;
		});
	
	return app.exec();
}

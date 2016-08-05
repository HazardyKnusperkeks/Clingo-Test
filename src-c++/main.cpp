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
	
	control.load("../program/graph_wg.lp");
	control.load("../program/mailbot.lp");
	
	control.ground({{"base", {{}}}}); //That looks nice, eh? ;)
	
	int horizon = 0, step = 0;
	
	Clingo::Symbol t(Clingo::Number(horizon));
	Clingo::SymbolSpan tSpan(&t, 1);
	Clingo::PartSpan tParts{{"state", tSpan}, {"transition", tSpan}, {"query", tSpan}};
	
	Clingo::Symbol query(Clingo::Function("query", tSpan, true));
	
	auto groundT = [&](void) noexcept {
			control.ground(tParts);
			control.assign_external(query, Clingo::TruthValue::True);
			return;
		};
	
	auto incrT = [&](void) noexcept {
			control.assign_external(query, Clingo::TruthValue::False);
			++horizon;
			t = Clingo::Number(horizon);
			query = Clingo::Function("query", tSpan, true);
			groundT();
			return;
		};
	
	groundT();
	
	QWidget widget;
	QGridLayout layout(&widget);
	QLabel status(&widget);
	QLabel result(&widget);
	QPushButton start("Start", &widget);
	QPushButton incr("Incr. t: 0", &widget);
	QLineEdit edit(&widget);
	QPushButton add("Add", &widget);
	QListView view(&widget);
	QStringListModel model;
	
	view.setModel(&model);
	
	layout.addWidget(&status, 0, 0);
	layout.addWidget(&start,  0, 1);
	layout.addWidget(&incr,   0, 2);
	layout.addWidget(&result, 1, 0, 1, -1);
	layout.addWidget(&edit,   2, 0, 1,  2);
	layout.addWidget(&add,    2, 2);
	layout.addWidget(&view,   3, 0, 1, -1);
	
	widget.show();
	
	return app.exec();
}

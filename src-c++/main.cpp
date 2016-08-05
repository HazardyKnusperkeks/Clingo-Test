#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QPushButton>
#include <QStringListModel>

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	
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

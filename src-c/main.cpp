#include <clingo.h>

#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QStringListModel>
#include <QTimer>

QStringListModel *itemModel = nullptr;

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	
	clingo_control_t *control = nullptr;
	clingo_logger_t *logger = nullptr;
	
	//Because this is just a test, there is no cleanup code in case of an error.
	switch ( clingo_control_new(nullptr, 0, logger, nullptr, 0xFFFF, &control) ) {
		case clingo_error_bad_alloc : return 1;
		case clingo_error_runtime   : return 2;
		case clingo_error_success   : break;
		default                     : return 3; //Should never happen.
	} //switch ( clingo_control_new(nullptr, 0, logger, nullptr, 0xFFFF, &control) )
	
	switch ( clingo_control_load(control, "../program/graph_wg.lp") ) {
		case clingo_error_bad_alloc : return 4;
		case clingo_error_runtime   : return 5; //Need to free control?!?
		case clingo_error_success   : break;
		default                     : return 6; //Should never happen.
	} //switch ( clingo_control_load(control, "../program/graph_wg.lp") )
	
	switch ( clingo_control_load(control, "../program/mailbot.lp") ) {
		case clingo_error_bad_alloc : return 7;
		case clingo_error_runtime   : return 8;
		case clingo_error_success   : break;
		default                     : return 9; //Should never happen.
	} //switch ( clingo_control_load(control, "../program/mailbot.lp") )
	
	clingo_symbol_t tParams[1];
	clingo_symbol_t commitParams[3];
	clingo_symbol_t set_eventParams[3];
	
	clingo_symbol_t *stateParams = tParams;
	clingo_symbol_t *transitionParams = tParams;
	clingo_symbol_t *queryParams = tParams;
	clingo_symbol_t *finalizeParams = tParams;
	
	clingo_part_t basePart[1] = {{"base", nullptr, 0}};
	switch ( clingo_control_ground(control, basePart, 1, nullptr, nullptr) ) {
		case clingo_error_bad_alloc : return 10;
		case clingo_error_success   : break;
		default                     : return 11; //Should never happen.
	} //switch ( clingo_control_ground(control, basePart, 1, nullptr, nullptr) )
	
	clingo_part_t tParts[] = {
			{"state", stateParams, sizeof(stateParams) / sizeof(clingo_symbol_t)},
			{"transition", transitionParams, sizeof(transitionParams) / sizeof(clingo_symbol_t)},
			{"query", queryParams, sizeof(queryParams) / sizeof(clingo_symbol_t)},
			{"finalize", finalizeParams, sizeof(finalizeParams) / sizeof(clingo_symbol_t)}
		};
	
	int t = 0;
	auto setT = [&control,&tParams,&commitParams,&set_eventParams,&t,&tParts](void) noexcept {
			clingo_symbol_create_number(t, &tParams[0]);
			clingo_symbol_create_number(t, &commitParams[2]);
			clingo_symbol_create_number(t, &set_eventParams[2]);
			
			clingo_control_ground(control, tParts, sizeof(tParts) / sizeof(clingo_part_t), nullptr, nullptr);
			return;
		};
	
	setT();
	++t;
	
	clingo_solve_async_t *async = nullptr;
	bool running = false;
	
	QWidget widget;
	QGridLayout layout(&widget);
	QLabel status(&widget);
	QLabel result(&widget);
	QPushButton stop("Stop",   &widget);
	QPushButton start("Start", &widget);
	QPushButton incr("Incr. t: 0", &widget);
	QLineEdit edit(&widget);
	QPushButton add("Add", &widget);
	QListView view(&widget);
	QStringListModel model;
	
	view.setModel(itemModel = &model);
	
	QTimer timer(&widget);
	
	layout.addWidget(&status, 0, 0);
	layout.addWidget(&stop,   0, 1);
	layout.addWidget(&start,  0, 2);
	layout.addWidget(&incr,   0, 3);
	layout.addWidget(&result, 1, 0, 1, -1);
	layout.addWidget(&edit,   2, 0, 1,  3);
	layout.addWidget(&add,    2, 3);
	layout.addWidget(&view,   3, 0, 1, -1);
	
	auto updateStatus = [&result,&running,&status](const QString& resultText = QString()) noexcept {
			status.setText(running ? "Solving" : "Finished");
			if ( !resultText.isEmpty() ) {
				result.setText(resultText);
			} //if ( !resultText.isEmpty() )
			return;
		};
	
	auto onModel    = [](clingo_model_t *model, void */*data*/, bool *goOn) noexcept -> clingo_error_t {
			*goOn = false;
			QStringList list;
			
			size_t atomsCount = 0;
			clingo_model_symbols_size(model, clingo_show_type_shown, &atomsCount);
			clingo_symbol_t atoms[atomsCount];
			clingo_model_symbols(model, clingo_show_type_shown, atoms, atomsCount);
			
			list.reserve(atomsCount);
			
			for ( const auto& atom : atoms ) {
				size_t strSize = 0;
				clingo_symbol_to_string_size(atom, &strSize);
				char str[strSize];
				clingo_symbol_to_string(atom, str, strSize);
				list.append(str);
			} //for ( const auto& atom : atoms )
			
			itemModel->setStringList(list);
			return 0;
		};
	auto onFinished = [](clingo_solve_result_bitset_t /*result*/, void *data) noexcept -> clingo_error_t {
			*static_cast<bool*>(data) = false;
			return 0;
		};
	
	auto stopSolving = [&async](void) noexcept {
			if ( async ) {
				clingo_solve_async_cancel(async);
			} //if ( async )
			return;
		};
	auto startSolving = [&async,&control,&onModel,&onFinished,&running,&timer,&updateStatus,&model](void) noexcept {
			if ( async ) {
				return;
			} //if ( async )
			model.setStringList(QStringList());
			running = true;
			updateStatus(" ");
			clingo_control_solve_async(control, onModel, nullptr, onFinished, &running, nullptr, 0, &async);
			return;
		};
		
	QObject::connect(&app,   &QApplication::aboutToQuit, stopSolving);
	QObject::connect(&stop,  &QPushButton::clicked,      stopSolving);
	QObject::connect(&start, &QPushButton::clicked,      startSolving);
	QObject::connect(&incr,  &QPushButton::clicked,      [&setT,&t,&incr](void) noexcept {
			incr.setText("Incr. t: " + QString::number(t));
			setT();
			++t;
			return;
		});
	QObject::connect(&incr,  &QPushButton::clicked,      &start, &QPushButton::click);
	QObject::connect(&add,   &QPushButton::clicked,      [&control,&edit,&t](void) noexcept {
			const QStringList text(edit.text().split(' '));
			edit.clear();
			
			switch ( text.size() ) {
				case 1 : {
					
					break;
				} //case 1
				case 3 : {
					clingo_symbol_t bring, bringArgs[2];
					
					for ( int i = 0; i < 2; ++i ) {
						clingo_symbol_create_id(text.at(i).toStdString().c_str(), true, bringArgs + i);
					} //for ( int i = 0; i < 2; ++i )
					clingo_symbol_create_function("bring", bringArgs, 2, true, &bring);
					
					clingo_symbol_t setEventParams[3] = {0, bring, 0};
					clingo_symbol_create_number(0, &setEventParams[0]);
					clingo_symbol_create_number(t, &setEventParams[2]);
					clingo_part_t setEvent = {"set_event", setEventParams, sizeof(setEventParams) / sizeof(clingo_symbol_t)};
					clingo_control_ground(control, &setEvent, 1, nullptr, nullptr);
					break;
				} //case 3
				default : break; //Print error
			} //switch ( text.size() )
			return;
		});
	QObject::connect(&add,   &QPushButton::clicked,      &start, &QPushButton::click);
	
	timer.setInterval(5);
	timer.setSingleShot(false);
	
	QObject::connect(&timer, &QTimer::timeout, [&async,&running,&updateStatus](void) noexcept {
			static bool old = running;
			if ( !running && old ) {
				clingo_solve_result_bitset_t result;
				clingo_solve_async_get(async, &result);
				updateStatus("Result: " + QString::number(result));
				async = nullptr;
			} //if ( !running && old )
			old = running;
			return;
		});
	
	timer.start();
	updateStatus();
	widget.show();
	
	const int ret = app.exec();
	
	clingo_control_free(control);
	return ret;
}

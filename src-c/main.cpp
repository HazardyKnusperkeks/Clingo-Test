#include <clingo.h>

#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>

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
	
	clingo_symbol_t stateParams[] = {0};
	clingo_symbol_create_number(0, &stateParams[0]);
	clingo_part_t parts[] = {{"base", nullptr, 0},
		{"state", stateParams, sizeof(stateParams) / sizeof(clingo_symbol_t)}};
	
	switch ( clingo_control_ground(control, parts, sizeof(parts) / sizeof(clingo_part_t), nullptr, nullptr) ) {
		case clingo_error_bad_alloc : return 10;
		case clingo_error_success   : break;
		default                     : return 11; //Should never happen.
	} //switch ( clingo_control_ground(control, parts, sizeof(parts) / sizeof(clingo_part_t), nullptr, nullptr) )
	
	clingo_solve_async_t *async = nullptr;
	bool running = false;
	
	QWidget widget;
	QGridLayout layout(&widget);
	QLabel status(&widget);
	QLabel result(&widget);
	QPushButton stop("Stop",   &widget);
	QPushButton start("Start", &widget);
	QTimer timer(&widget);
	
	layout.addWidget(&status, 0, 0);
	layout.addWidget(&stop,   0, 1);
	layout.addWidget(&start,  0, 2);
	layout.addWidget(&result, 1, 0, 1, -1);
	
	auto updateStatus = [&result,&running,&status](const QString& resultText = QString()) noexcept {
			status.setText(running ? "Solving" : "Finished");
			if ( !resultText.isEmpty() ) {
				result.setText(resultText);
			} //if ( !resultText.isEmpty() )
			return;
		};
	
	auto onModel    = [](clingo_model_t *model, void */*data*/, bool */*goOn*/) noexcept -> clingo_error_t {
			clingo_error_t ret = 0;
			clingo_symbol_t *atoms = NULL;
			size_t atoms_n;
			clingo_symbol_t const *it, *ie;
			char *str = NULL;
			size_t str_n = 0;
			// determine the number of (shown) symbols in the model
			if ((ret = clingo_model_symbols_size(model, clingo_show_type_shown, &atoms_n))) { goto error; }
			// allocate required memory to hold all the symbols
			if (!(atoms = (clingo_symbol_t*)malloc(sizeof(*atoms) * atoms_n))) { goto error; }
			// retrieve the symbols in the model
			if ((ret = clingo_model_symbols(model, clingo_show_type_shown, atoms, atoms_n))) { goto error; }
			printf("Model:");
			for (it = atoms, ie = atoms + atoms_n; it != ie; ++it) {
			size_t n;
			char *str_new;
			// determine size of the string representation of the next symbol in the model
			if ((ret = clingo_symbol_to_string_size(*it, &n))) { goto error; }
			if (str_n < n) {
			// allocate required memory to hold the symbol's string
			if (!(str_new = (char*)realloc(str, sizeof(*str) * n))) { goto error; }
			str = str_new;
			str_n = n;
			}
			// retrieve the symbol's string
			if (clingo_symbol_to_string(*it, str, n)) { goto error; }
			printf(" %s", str);
			}
			printf("\n");
			goto out;
			error:
			if (!ret) { ret = clingo_error_unknown; }
			out:
			if (atoms) { free(atoms); }
			if (str)   { free(str); }
			return ret;
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
	auto startSolving = [&async,&control,&onModel,&onFinished,&running,&updateStatus](void) noexcept {
			if ( async ) {
				return;
			} //if ( async )
			clingo_control_solve_async(control, onModel, nullptr, onFinished, &running, nullptr, 0, &async);
			running = true;
			updateStatus();
			return;
		};
		
	QObject::connect(&app,   &QApplication::aboutToQuit, stopSolving);
	QObject::connect(&stop,  &QPushButton::clicked,      stopSolving);
	QObject::connect(&start, &QPushButton::clicked,      startSolving);
	
	timer.setInterval(50);
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

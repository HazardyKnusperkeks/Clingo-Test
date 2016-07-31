#include <clingo.h>

int main(/*int argc, char *argv[]*/) {
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
	
//	clingo_solve_result_bitset_t solve_ret;
//	clingo_solve_async_t *async = nullptr;
	
	clingo_control_free(control);
	return 0;
}

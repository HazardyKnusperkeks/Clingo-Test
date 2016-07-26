#include <clingo/clingocontrol.hh>

int main(/*int argc, char *argv[]*/) {
	DefaultGringoModule gringoModule;
	Gringo::Scripts scripts(gringoModule);

	Clasp::ClaspFacade claspFacade;

	Clasp::Cli::ClaspCliConfig cliConfig;

	ClingoControl control(scripts, true, &claspFacade, cliConfig, nullptr, nullptr, nullptr, 65535);
	return 0;
}

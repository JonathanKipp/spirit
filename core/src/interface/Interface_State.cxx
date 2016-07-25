#include "Interface_State.h"

#include "Version.h"
#include "Logging.h"
#include "IO.h"
#include "Configurations.h"
#include "Configuration_Chain.h"


using namespace Utility;

extern "C" State * setupState(const char * config_file)
{
    // Create the State
    State *state = new State();
    
    // Log
    Log.Send(Log_Level::ALL, Log_Sender::ALL,  "=====================================================");
    Log.Send(Log_Level::ALL, Log_Sender::ALL,  "============ MonoSpin State Initialising ============");
    Log.Send(Log_Level::ALL, Log_Sender::ALL,  "================= Version:  " + std::string(VERSION));
    Log.Send(Log_Level::INFO, Log_Sender::ALL, "================= Revision: " + std::string(VERSION_REVISION));
    Log.Send(Log_Level::ALL, Log_Sender::ALL,  "=====================================================");
    
    try
    {
        //---------------------- Read Log Levels ----------------------------------------
        IO::Log_Levels_from_Config(config_file);
        //-------------------------------------------------------------------------------

        //---------------------- initialize spin_systems --------------------------------
        // Create a system according to Config
        state->active_image = IO::Spin_System_from_Config(config_file);
        //-------------------------------------------------------------------------------

        //---------------------- set images' configurations -----------------------------
        // Parameters
        double dir[3] = { 0,0,1 };
        std::vector<double> pos = { 14.5, 14.5, 0 };
        // First image is homogeneous with a Skyrmion at pos
        Configurations::Random(*state->active_image);
        //Configurations::Skyrmion(*s1, pos, 6.0, 1.0, -90.0, false, false, false, false);
        //-------------------------------------------------------------------------------


        //----------------------- initialize spin system chain --------------------------
        // Get parameters
        auto params_gneb = std::shared_ptr<Data::Parameters_GNEB>(IO::GNEB_Parameters_from_Config(config_file));
        // Create the chain
        auto sv = std::vector<std::shared_ptr<Data::Spin_System>>();
        sv.push_back(state->active_image);
        state->active_chain = std::shared_ptr<Data::Spin_System_Chain>(new Data::Spin_System_Chain(sv, params_gneb, false));
        //-------------------------------------------------------------------------------
    }
	catch (Exception ex)
    {
		if (ex == Exception::System_not_Initialized)
			Log.Send(Utility::Log_Level::SEVERE, Utility::Log_Sender::IO, std::string("System not initialized - Terminating."));
		else if (ex == Exception::Simulated_domain_too_small)
			Log.Send(Utility::Log_Level::SEVERE, Utility::Log_Sender::ALL, std::string("CreateNeighbours:: Simulated domain is too small"));
		else if (ex == Exception::Not_Implemented)
			Log.Send(Utility::Log_Level::SEVERE, Utility::Log_Sender::ALL, std::string("Tried to use function which has not been implemented"));
		else
			Log.Send(Utility::Log_Level::SEVERE, Utility::Log_Sender::ALL, std::string("Unknown exception!"));
	}

    // active images
    state->idx_active_chain = 0;
    state->idx_active_image = 0;

    // Info
    state->noc = 1;
    state->noi = 1;
    state->nos = state->active_image->nos;

    // Solvers
    state->solvers_llg = std::vector<std::vector<std::shared_ptr<Engine::Solver_LLG>>>(state->noc, std::vector<std::shared_ptr<Engine::Solver_LLG>>(state->noi));
    state->solvers_gneb = std::vector<std::shared_ptr<Engine::Solver_GNEB>>(state->noi);
    state->solvers_mmf = std::vector<std::shared_ptr<Engine::Solver_MMF>>(state->noi);

    // Log
    Log.Send(Log_Level::ALL, Log_Sender::ALL, "=====================================================");
    Log.Send(Log_Level::ALL, Log_Sender::ALL, "============ MonoSpin State Initialised =============");
    Log.Send(Log_Level::ALL, Log_Sender::ALL, "=====================================================");
    
    // Return
    return state;
}
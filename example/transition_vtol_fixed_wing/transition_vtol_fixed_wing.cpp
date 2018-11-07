#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>
#include <dronecode_sdk/action.h>
#include <dronecode_sdk/dronecode_sdk.h>
#include <dronecode_sdk/telemetry.h>

using std::this_thread::sleep_for;
using std::chrono::seconds;

using namespace dronecode_sdk;

int main(int /* argc */, char** /* argv */)
{
    DronecodeSDK dc;

    const ConnectionResult connection_result = dc.add_udp_connection(14540);

    if (connection_result != ConnectionResult::SUCCESS) {
        std::cout << "Connection failed: " << connection_result_str(connection_result) << std::endl;
        return 1;
    }

    while (!dc.system().has_autopilot()) {
        sleep_for(seconds(1));
        std::cout << "Waiting for system to connect." << std::endl;
    }

    System &system = dc.system();

    auto telemetry = std::make_shared<Telemetry>(system);
    auto action = std::make_shared<Action>(system);

    while (!telemetry->health_all_ok()) {
        std::cout << "Waiting for vehicle to be ready to arm..." << std::endl;
        sleep_for(seconds(1));
    }

    const ActionResult arm_result = action->arm();

    if (arm_result != ActionResult::SUCCESS) {
        std::cout << "Arming failed: " << action_result_str(arm_result) << std::endl;
        return 1;
    }

    action->takeoff();

    // Wait while it takes off.
    sleep_for(seconds(10));

    action->transition_to_fixedwing();

    // Let it transition and start loitering.
    sleep_for(seconds(30));

    action->goto_location(47.3633001, 8.5428515, NAN, NAN);

    // Let it fly South for a bit.
    sleep_for(seconds(15));

    action->transition_to_multicopter();

    // Wait for the transition to be carried out.
    sleep_for(seconds(5));

    action->land();

    // Wait until disarmed.
    while (telemetry->armed()) {
        std::cout << "Waiting for vehicle to land and disarm." << std::endl;
        sleep_for(seconds(1));
    }

    return 0;
}

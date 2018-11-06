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

    // We need an autopilot connected to start.
    while (!dc.system().has_autopilot()) {
        sleep_for(seconds(1));
        std::cout << "Waiting for system to connect." << std::endl;
    }

    // Get system and plugins.
    System &system = dc.system();
    auto telemetry = std::make_shared<Telemetry>(system);
    auto action = std::make_shared<Action>(system);

    // We want to listen to the altitude of the drone at 1 Hz.
    const Telemetry::Result set_rate_result = telemetry->set_rate_position(1.0);
    if (set_rate_result != Telemetry::Result::SUCCESS) {
        std::cout << "Setting rate failed: " << Telemetry::result_str(set_rate_result) << std::endl;
        return 1;
    }

    // Set up callback to monitor altitude.
    telemetry->position_async([](Telemetry::Position position) {
        std::cout << "Altitude: " << position.relative_altitude_m << " m" << std::endl;
    });

    // Wait until we are ready to arm.
    while (!telemetry->health_all_ok()) {
        std::cout << "Waiting for vehicle to be ready to arm..." << std::endl;
        sleep_for(seconds(1));
    }

    // Arm vehicle
    std::cout << "Arming." << std::endl;
    const ActionResult arm_result = action->arm();
    if (arm_result != ActionResult::SUCCESS) {
        std::cout << "Arming failed: " << action_result_str(arm_result) << std::endl;
        return 1;
    }

    // Take off
    std::cout << "Taking off." << std::endl;
    const ActionResult takeoff_result = action->takeoff();
    if (takeoff_result != ActionResult::SUCCESS) {
        std::cout << "Takeoff failed:n" << action_result_str(takeoff_result) << std::endl;
        return 1;
    }

    // Wait while it takes off.
    sleep_for(seconds(10));

    std::cout << "Transition to fixedwing." << std::endl;
    const ActionResult fw_result = action->transition_to_fixedwing();
    if (fw_result != ActionResult::SUCCESS) {
        std::cout << "Transition to fixed wing failed: " << action_result_str(fw_result) << std::endl;
        return 1;
    }

    // Let it transition and start loitering.
    sleep_for(seconds(30));

    // Send it South.
    std::cout << "Sending it to location." << std::endl;
    // We pass latitude and longitude but leave altitude and yaw unset by passing NAN.
    const ActionResult goto_result = action->goto_location(47.3633001, 8.5428515, NAN, NAN);
    if (goto_result != ActionResult::SUCCESS) {
        std::cout << "Goto command failed: " << action_result_str(goto_result) << std::endl;
        return 1;
    }

    // Let it fly South for a bit.
    sleep_for(seconds(15));

    // Let's stop before reaching the goto point and go back to hover.
    std::cout << "Transition back to multicopter..." << std::endl;
    const ActionResult mc_result = action->transition_to_multicopter();
    if (mc_result != ActionResult::SUCCESS) {
        std::cout << "Transition to multi copter failed: " << action_result_str(mc_result) << std::endl;
        return 1;
    }

    // Wait for the transition to be carried out.
    sleep_for(seconds(5));

    // Now just land here.
    std::cout << "Landing..." << std::endl;
    const ActionResult land_result = action->land();
    if (land_result != ActionResult::SUCCESS) {
        std::cout << "Land failed: " << action_result_str(land_result) << std::endl;
        return 1;
    }

    // Wait until disarmed.
    while (telemetry->armed()) {
        std::cout << "Waiting for vehicle to land and disarm." << std::endl;
        sleep_for(seconds(1));
    }

    std::cout << "Disarmed, exiting." << std::endl;
    return 0;
}

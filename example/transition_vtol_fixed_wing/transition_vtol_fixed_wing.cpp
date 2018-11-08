#include <dronecode_sdk/action.h>
#include <dronecode_sdk/dronecode_sdk.h>
#include <dronecode_sdk/telemetry.h>

using namespace dronecode_sdk;


int main(int /* argc */, char** /* argv */)
{
    DronecodeSDK dc;

    dc.add_udp_connection(14540);

    System &system = dc.system();

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
    sleep_for(seconds(10));
    action->transition_to_fixedwing();
    sleep_for(seconds(30));
    action->goto_location(47.3633001, 8.5428515, NAN, NAN);
    sleep_for(seconds(15));
    action->transition_to_multicopter();
    sleep_for(seconds(5));
    action->land();

    // Wait until disarmed.
    while (telemetry->armed()) {
        std::cout << "Waiting for vehicle to land and disarm." << std::endl;
        sleep_for(seconds(1));
    }

    return 0;
}
